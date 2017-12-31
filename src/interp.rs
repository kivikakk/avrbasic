use exec::{add_var, VarValue};

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

#[derive(Debug, PartialEq, Clone)]
pub enum TokenType {
    Number,
    Label,
    BinOp,
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

impl TokenType {
    fn expect<'i>(&self, r: &TokenType) -> Result<(), InterpError<'i>> {
        if self != r {
            Err(InterpError::BadTokenType((*self).clone()))
        } else {
            Ok(())
        }
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

        let itt = Self::tt(self.v[0], None);

        let mut len = 0;
        while len < self.v.len() && self.v[len] != b' ' && Self::tt(self.v[len], Some(&itt)) == itt
        {
            len += 1;
        }

        let t = &self.v[..len];
        self.v = &self.v[len..];

        Some((t, itt))
    }
}

impl<'i> Splitter<'i> {
    fn tt(c: u8, s: Option<&TokenType>) -> TokenType {
        match s {
            None | Some(&TokenType::Number) | Some(&TokenType::BinOp) => match c {
                b'0'...b'9' => TokenType::Number,
                b'A'...b'Z' => TokenType::Label,
                b'+' | b'-' | b'*' | b'/' | b'%' | b'=' => TokenType::BinOp,
                c => panic!("got char {:?}", c as char),
            },
            Some(&TokenType::Label) => match c {
                b'0'...b'9' | b'A'...b'Z' | b'!' | b'$' | b'#' | b'%' => TokenType::Label,
                b'+' | b'-' | b'*' | b'/' | b'=' => TokenType::BinOp,
                _ => panic!(),
            },
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
    tt.expect(&TokenType::Number)?;
    let n = VarValue::Integer(interp_number(n)?);

    if s.check_end()? {
        return Ok(n);
    }

    let (_op, tt) = s.0.next().ok_or("missing op")?;
    tt.expect(&TokenType::BinOp)?;
    // let op = interp_binop(op)?;

    // let n2 = interp_expr(s)?;

    //Ok(Expr::BinOp(op, &n, &n2))
    Err("no".into())
}

fn interp_number(mut s: &[u8]) -> Result<i16, InterpError> {
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
fn interp_binop(s: &[u8]) -> Result<BinOp, InterpError> {
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
    use exec::get_var;

    #[test]
    fn interp_let() {
        assert_eq!(get_var([b'X', 0], b'%'), VarValue::Integer(0));
        assert_eq!(interp(b"LET X% = 1"), Ok(()));
        assert_eq!(get_var([b'X', 0], b'%'), VarValue::Integer(1));
    }
}
