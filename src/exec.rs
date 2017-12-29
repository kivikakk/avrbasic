use synced::Synced;

#[derive(Debug, PartialEq)]
pub enum VarValue {
    Integer(i16),    // %
    Single(f32),     // !
    Double(f64),     // #
    String(u8, u16), // $
}

pub type Var = ([u8; 2], VarValue);

#[cfg(target_arch = "avr")]
static VHEAP: Synced<*mut u8> = unsafe { Synced::new(0 as *mut _) };
static SHEAP: Synced<*mut u8> = unsafe { Synced::new(0x400 as *mut _) };

#[inline(always)]
pub fn run() {}

fn add_var(vn: [u8; 2], val: VarValue) {}

fn get_var(vn: [u8; 2], t: u8) -> VarValue {
    VarValue::Integer(0)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add() {
        add_var([b'A', b'\0'], VarValue::Integer(105));
        assert_eq!(get_var([b'A', b'\0'], b'%'), VarValue::Integer(105));
    }
}
