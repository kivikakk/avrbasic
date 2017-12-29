#[derive(Debug, PartialEq)]
pub enum VarValue {
    Integer(i16),    // %
    Single(f32),     // !
    Double(f64),     // #
    String(u8, u16), // $
}

pub type Var = ([u8; 2], VarValue);

#[cfg(target_arch = "avr")]
mod heaps {
    use synced::Synced;

    pub static VHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new(0 as *mut _)) };
    pub static SHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new(0x400 as *mut _)) };
}

#[cfg(not(target_arch = "avr"))]
mod heaps {
    use synced::Synced;

    static mut _VHEAP: [u8; 0x400] = [0; 0x400];
    lazy_static! {
        pub static ref VHEAP: Synced<*mut u8> = unsafe {
            Synced::new(&mut _VHEAP as *const _ as *mut _)
        };
    }
}

use self::heaps::VHEAP;

use arduino::{DDRB, PORTB};

pub fn run() {
    unsafe { DDRB.write_volatile(0xFF) };

    loop {
        add_var([b'A', 0], VarValue::Integer(105));
        let v = get_var([b'A', 0], b'%');

        match v {
            VarValue::Integer(0) => unsafe {
                PORTB.write_volatile(0xFF);
            },
            VarValue::Integer(0) => unsafe {
                PORTB.write_volatile(0xFE);
            },
            _ => (),
        }
    }
}

fn add_var(vn: [u8; 2], val: VarValue) {
    unsafe {
        VHEAP.write_volatile(vn[0]);
        VHEAP.add(1).write_volatile(vn[1]);
        match val {
            VarValue::Integer(i) => {
                VHEAP.add(2).write_volatile(1);
                VHEAP.add(3).write_volatile(((i >> 8) & 0xFF) as u8);
                VHEAP.add(4).write_volatile((i & 0xFF) as u8);
            }
            _ => (),
        }
    }
}

fn get_var(vn: [u8; 2], t: u8) -> VarValue {
    unsafe {
        let a1 = VHEAP.read();
        if a1 == vn[0] {
            let a2 = VHEAP.add(1).read();
            if a2 == vn[1] {
                match VHEAP.add(2).read() {
                    1 => if t == b'%' {
                        let a = VHEAP.add(3).read();
                        let b = VHEAP.add(4).read();
                        return VarValue::Integer(((a as u16) << 8 | (b as u16)) as i16);
                    },
                    _ => panic!(),
                }
            }
        }
        panic!();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add() {
        add_var([b'A', 0], VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(0, 0));

        add_var([b'A', 0], VarValue::String(1, 1));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(1, 1));
    }
}
