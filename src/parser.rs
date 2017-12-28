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

#[derive(PartialEq)]
enum TokenType {
    Number,
    Label,
    BinOp,
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
        while len < self.v.len() && self.v[len] != b' ' && Self::tt(self.v[len]) != itt {
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
            b'+' | b'-' | b'*' | b'/' | b'%' => TokenType::BinOp,
            _ => panic!(),
        }
    }
}

pub fn parse<'i>(i: &'i [u8]) -> Result<Statement<'i>, &'static str> {
    let mut s = Splitter { v: i };
    let t = s.next().ok_or("no statement")?;

    if t == b"LET" {
        let v = t.next().ok_or("missing LET variable")?;
        match v {}
    }

    Err("reached end of parse")
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
