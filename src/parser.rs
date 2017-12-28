use core::iter::Peekable;

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
    Number(i32),

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
}

pub fn parse<'i>(i: &'i [u8]) -> Result<Statement<'i>, ParseError> {
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

fn parse_expr<'i>(mut s: PSplitter<'i>) -> Result<Expr<'i>, ParseError> {
    Err("blah".into())
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
