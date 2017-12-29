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
pub enum ParseError<'i> {
    Empty,
    UnknownToken(&'i [u8]),
    BadTokenType(TokenType),
    Expected(&'static [u8]),
    Other(&'static str),
}

impl<'i> From<&'static str> for ParseError<'i> {
    fn from(s: &'static str) -> ParseError<'i> {
        ParseError::Other(s)
    }
}

impl TokenType {
    fn expect<'i>(&self, r: TokenType) -> Result<(), ParseError<'i>> {
        if *self != r {
            Err(ParseError::BadTokenType((*self).clone()))
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

        let itt = Self::tt(self.v[0]);

        let mut len = 0;
        while len < self.v.len() && self.v[len] != b' ' && Self::tt(self.v[len]) == itt {
            len += 1;
        }

        let t = &self.v[..len];
        self.v = &self.v[len..];

        Some((t, itt))
    }
}

impl<'i> Splitter<'i> {
    fn tt(c: u8) -> TokenType {
        match c {
            b'0'...b'9' => TokenType::Number,
            b'A'...b'Z' => TokenType::Label,
            b'+' | b'-' | b'*' | b'/' | b'%' | b'=' => TokenType::BinOp,
            _ => panic!(),
        }
    }

    fn psplitter(self) -> PSplitter<'i> {
        PSplitter(self.peekable())
    }
}

struct PSplitter<'i>(pub Peekable<Splitter<'i>>);

impl<'i> PSplitter<'i> {
    fn expect(&mut self, s: &'static [u8]) -> Result<(), ParseError<'i>> {
        if !self.check(s)? {
            Err(ParseError::Expected(s))
        } else {
            Ok(())
        }
    }

    fn check(&mut self, s: &'static [u8]) -> Result<bool, ParseError<'i>> {
        let &(t, _) = self.0.peek().ok_or(ParseError::Empty)?;
        if t == s {
            self.0.next();
            Ok(true)
        } else {
            Ok(false)
        }
    }

    #[allow(dead_code)]
    fn expect_end(&mut self) -> Result<(), ParseError<'i>> {
        if !self.check_end()? {
            Err(ParseError::Expected(b"end of statement"))
        } else {
            Ok(())
        }
    }

    fn check_end(&mut self) -> Result<bool, ParseError<'i>> {
        Ok(self.0.peek().is_none())
    }
}

pub fn parse<'i>(i: &'i [u8]) -> Result<Statement<'i>, ParseError<'i>> {
    let mut s = Splitter { v: i }.psplitter();

    if s.check(b"LET")? {
        let (v, tt) = s.0.next().ok_or("missing LET variable")?;
        tt.expect(TokenType::Label)?;

        s.expect(b"=")?;

        let r = parse_expr(s)?;

        Ok(Statement::Let(v, r))
    } else {
        Err(ParseError::UnknownToken(s.0.next().unwrap().0))
    }
}

fn parse_expr<'i>(mut s: PSplitter<'i>) -> Result<Expr<'i>, ParseError<'i>> {
    let (n, tt) = s.0.next().ok_or("missing expr")?;
    tt.expect(TokenType::Number)?;
    let n = Expr::Number(parse_number(n)?);

    if s.check_end()? {
        return Ok(n);
    }

    let (_op, tt) = s.0.next().ok_or("missing op")?;
    tt.expect(TokenType::BinOp)?;
    // let op = parse_binop(op)?;

    // let n2 = parse_expr(s)?;

    //Ok(Expr::BinOp(op, &n, &n2))
    Err("no".into())
}

fn parse_number<'i>(mut s: &'i [u8]) -> Result<i16, ParseError<'i>> {
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
        n = (n * 10) + (c - b'0') as i16;
        s = &s[1..];
    }

    Ok(n * if neg { -1 } else { 1 })
}

#[allow(dead_code)]
fn parse_binop<'i>(s: &'i [u8]) -> Result<BinOp, ParseError<'i>> {
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

    #[test]
    fn parse_let() {
        assert_eq!(
            parse(b"LET X = 1"),
            Ok(Statement::Let(b"X", Expr::Number(1)))
        );
    }
}
