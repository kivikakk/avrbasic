use synced::Synced;

pub enum VarValue {
    Integer(i16),
    Single(f32),
    Double(f64),
    String(u8, u16),
}

pub type Var = ([u8; 2], VarValue);

#[cfg(target_arch = "avr")]
static VHEAP: Synced<*mut u8> = unsafe { Synced::new(0 as *mut _) };
static SHEAP: Synced<*mut u8> = unsafe { Synced::new(0x400 as *mut _) };

#[inline(always)]
pub fn run() {}

fn add_var(vn: [u8; 2], val: VarValue) {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add() {
        add_var([b'A', b'\0'], VarValue::Integer(105));
    }
}
