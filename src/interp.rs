use exec::{add_var, BoxedString, VarValue};

#[cfg(target_arch = "avr")]
use core::iter::Peekable;
#[cfg(not(target_arch = "avr"))]
use std::iter::Peekable;

#[derive(Debug, PartialEq)]
pub enum Statement<'i> {
    Let(&'i [u8], Expr<'i>),
    // Data(),
    If(),
    For(),
    // While(),
    // Repeat(),
    // Do(),
    Goto(),
    // Gosub(),
    // OnGoto(),
    // OnGosub(),
    Print(),
    Input(),
    // Tab(),
    End(),
}

#[derive(Debug, PartialEq)]
pub enum BinOp {
    Add,
    Subtract,
    Equal,
}

#[derive(Debug, PartialEq)]
pub enum Expr<'i> {
    Label(&'i [u8]),
    Number(i16),

    FnCall(&'i [u8], &'i [Expr<'i>]),
    // .-------------^
    // this should not be 'i
    BinOp(BinOp, &'i Expr<'i>, &'i Expr<'i>),
}

#[derive(Debug, PartialEq)]
pub enum InterpError<'i> {
    Empty,
    UnknownToken(&'i [u8]),
    BadTokenType(TokenType),
    Expected(&'static [u8]),
    Other(&'static str),
}

impl<'i> From<&'static str> for InterpError<'i> {
    fn from(s: &'static str) -> InterpError<'i> {
        InterpError::Other(s)
    }
}

#[derive(Debug, PartialEq, Clone)]
pub enum TokenType {
    Number,
    Label,
    BinOp,
    String,
}

impl TokenType {
    fn expect<'i>(&self, r: &TokenType) -> Result<(), InterpError<'i>> {
        if !self.check(r) {
            Err(InterpError::BadTokenType((*self).clone()))
        } else {
            Ok(())
        }
    }

    fn check(&self, r: &TokenType) -> bool {
        self == r
    }
}

struct Splitter<'i> {
    v: &'i [u8],
}

impl<'i> Iterator for Splitter<'i> {
    type Item = (&'i [u8], TokenType);

    #[inline]
    fn next(&mut self) -> Option<(&'i [u8], TokenType)> {
        while !self.v.is_empty() && self.v[0] == b' ' {
            self.v = &self.v[1..];
        }

        if self.v.is_empty() {
            return None;
        }

        let itt = match Self::tt(self.v[0], None) {
            Ok(tt) => tt,
            Err(_) => return None,
        };

        let mut len = 0;
        while len < self.v.len() && self.v[len] != b' ' && match Self::tt(self.v[len], Some(&itt)) {
            Ok(tt) => tt == itt,
            Err(_) => false,
        } {
            len += 1;
        }

        let t = &self.v[..len];
        self.v = &self.v[len..];

        Some((t, itt))
    }
}

impl<'i> Splitter<'i> {
    fn tt(c: u8, s: Option<&TokenType>) -> Result<TokenType, InterpError> {
        #[cfg_attr(target_arch = "avr", allow(unused_variables))]
        match s {
            None | Some(&TokenType::Number) | Some(&TokenType::BinOp) => match c {
                b'0'...b'9' => Ok(TokenType::Number),
                b'A'...b'Z' => Ok(TokenType::Label),
                b'+' | b'-' | b'*' | b'/' | b'%' | b'=' => Ok(TokenType::BinOp),
                b'"' => Ok(TokenType::String),
                _ => Err("got bad char".into()),
            },
            Some(&TokenType::Label) => match c {
                b'0'...b'9' | b'A'...b'Z' | b'!' | b'$' | b'#' | b'%' => Ok(TokenType::Label),
                b'+' | b'-' | b'*' | b'/' | b'=' => Ok(TokenType::BinOp),
                _ => Err("got bad char in label".into()),
            },
            Some(&TokenType::String) => Ok(TokenType::String),
        }
    }

    fn psplitter(self) -> PSplitter<'i> {
        PSplitter(self.peekable())
    }
}

struct PSplitter<'i>(pub Peekable<Splitter<'i>>);

impl<'i> PSplitter<'i> {
    fn expect(&mut self, s: &'static [u8]) -> Result<(), InterpError<'i>> {
        if !self.check(s)? {
            Err(InterpError::Expected(s))
        } else {
            Ok(())
        }
    }

    fn check(&mut self, s: &'static [u8]) -> Result<bool, InterpError<'i>> {
        let &(t, _) = self.0.peek().ok_or(InterpError::Empty)?;
        if t == s {
            self.0.next();
            Ok(true)
        } else {
            Ok(false)
        }
    }

    #[allow(dead_code)]
    fn expect_end(&mut self) -> Result<(), InterpError<'i>> {
        if !self.check_end()? {
            Err(InterpError::Expected(b"end of statement"))
        } else {
            Ok(())
        }
    }

    fn check_end(&mut self) -> Result<bool, InterpError<'i>> {
        Ok(self.0.peek().is_none())
    }
}

pub fn interp(i: &[u8]) -> Result<(), InterpError> {
    let mut s = Splitter { v: i }.psplitter();

    if s.check(b"LET")? {
        let (v, tt) = s.0.next().ok_or("missing LET variable")?;
        tt.expect(&TokenType::Label)?;

        match v[v.len() - 1] {
            b'%' | b'$' => (),
            _ => return Err(InterpError::Other("expected var type")),
        }

        s.expect(b"=")?;

        let r = interp_expr(s)?;

        let vn: [u8; 2] = [v[0], if v.len() > 2 { v[1] } else { 0 }];
        match (v[v.len() - 1], &r) {
            (b'%', &VarValue::Integer(..)) | (b'$', &VarValue::String(..)) => add_var(vn, &r),
            _ => return Err(InterpError::Other("bad return value")),
        };

        Ok(())
    } else {
        Err(InterpError::UnknownToken(s.0.next().unwrap().0))
    }
}

fn interp_expr(mut s: PSplitter) -> Result<VarValue, InterpError> {
    let (n, tt) = s.0.next().ok_or("missing expr")?;

    if tt.check(&TokenType::Number) {
        let n = parse_number(n)?;

        if s.check_end()? {
            return Ok(VarValue::Integer(n));
        }

        let (op, tt) = s.0.next().ok_or("missing op")?;
        tt.expect(&TokenType::BinOp)?;
        let op = parse_binop(op)?;

        let n2 = interp_expr(s)?;

        match n2 {
            VarValue::Integer(n2) => Ok(VarValue::Integer(match op {
                BinOp::Add => n + n2,
                BinOp::Subtract => n - n2,
                BinOp::Equal => if n == n2 {
                    1
                } else {
                    0
                },
            })),
            _ => Err("bad add".into()),
        }
    } else if tt.check(&TokenType::String) {
        let n = &n[1..n.len() - 1];
        let s = BoxedString::new(n);
        Ok(VarValue::String(s))
    } else {
        Err(InterpError::BadTokenType(tt))
    }
}

fn parse_number(mut s: &[u8]) -> Result<i16, InterpError> {
    if s.is_empty() {
        return Err("no number".into());
    }

    let neg = if s[0] == b'-' {
        s = &s[1..];
        true
    } else {
        false
    };

    let mut n: i16 = 0;
    for c in s {
        n = (n * 10) + i16::from(c - b'0');
        s = &s[1..];
    }

    Ok(n * if neg { -1 } else { 1 })
}

#[allow(dead_code)]
fn parse_binop(s: &[u8]) -> Result<BinOp, InterpError> {
    if s.is_empty() {
        return Err("no binop".into());
    }

    if s.len() > 1 {
        return Err("too much binop".into());
    }

    Ok(match s[0] {
        b'+' => BinOp::Add,
        b'=' => BinOp::Equal,
        b'-' => BinOp::Subtract,
        _ => return Err("invalid binop".into()),
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use exec::{clear_vars, get_var};

    #[test]
    fn interp_let() {
        clear_vars();
        assert_eq!(get_var([b'X', 0], b'%'), None);
        assert_eq!(interp(b"LET X% = 1"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), Some(VarValue::Integer(1)));
        assert_eq!(interp(b"LET X$ = \"ROBOT\""), Ok(()));
        let v = get_var([b'X', 0], b'$');
        match v {
            Some(VarValue::String(ref bs)) => assert_eq!(bs.value(), b"ROBOT"),
            _ => panic!("no string"),
        }
    }

    #[test]
    fn interp_binop() {
        clear_vars();
        assert_eq!(get_var([b'X', 0], b'%'), None);
        assert_eq!(interp(b"LET X% = 1 + 2"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), Some(VarValue::Integer(3)));
        assert_eq!(interp(b"LET X% = 1 - 2"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), Some(VarValue::Integer(-1)));
        assert_eq!(interp(b"LET X% = 1 = 2"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), Some(VarValue::Integer(0)));
        assert_eq!(interp(b"LET X% = 2 = 2"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), Some(VarValue::Integer(1)));

        assert_eq!(get_var([b'X', 0], b'$'), None);
        assert_eq!(interp(b"LET X$ = \"ROB\" + \"OTS\""), Ok(()));
        let v = get_var([b'X', 0], b'$');
        match v {
            Some(VarValue::String(ref bs)) => assert_eq!(bs.value(), b"ROBOTS"),
            _ => panic!("no string"),
        }
    }
}
