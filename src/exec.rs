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

use arduino::{DDRB, PINB, PORTB};

pub fn run() {
    // TODO: abstract away any port writes so I can
    // `cargo run' on amd64.

    unsafe {
        DDRB.write_volatile(0x01);
        VHEAP.write_bytes(0, 0x400);
    }

    loop {
        add_var([b'A', 0], VarValue::Integer(105));
        let c = unsafe { PINB.read_volatile() };
        let v = get_var([c, 0], b'%');

        match v {
            VarValue::Integer(0) => unsafe {
                PORTB.write_volatile(0xFF);
            },
            VarValue::Integer(1) => unsafe {
                PORTB.write_volatile(0xFE);
            },
            _ => (),
        }
    }
}

fn add_var(vn: [u8; 2], val: VarValue) {
    unsafe {
        let mut h = **VHEAP;

        loop {
            if h.read() != 0 {
                let t = h.add(2).read();
                match t {
                    1 => h = h.add(5),
                    4 => h = h.add(6),
                    _ => panic!("lol?"),
                }
                continue;
            }

            h.write(vn[0]);
            h.add(1).write(vn[1]);
            match val {
                VarValue::Integer(i) => {
                    h.add(2).write(1);
                    h.add(3).write((i & 0xFF) as u8);
                    h.add(4).write(((i >> 8) & 0xFF) as u8);
                }
                VarValue::String(l, o) => {
                    h.add(2).write(4);
                    h.add(3).write(l);
                    h.add(4).write(o as u8);
                    h.add(5).write(((o >> 8) & 0xFF) as u8);
                }
                _ => panic!("ha?"),
            }

            break;
        }
    }
}

fn get_var(vn: [u8; 2], t: u8) -> VarValue {
    unsafe {
        let h = **VHEAP;

        let a1 = h.read();
        if a1 == vn[0] {
            let a2 = h.add(1).read();
            if a2 == vn[1] {
                match h.add(2).read() {
                    1 => if t == b'%' {
                        let b = h.add(3).read();
                        let a = h.add(4).read();
                        return VarValue::Integer(((a as u16) << 8 | (b as u16)) as i16);
                    },
                    4 => if t == b'$' {
                        let s = h.add(3).read();
                        let b = h.add(4).read();
                        let a = h.add(5).read();
                        return VarValue::String(s, (((a as u16) << 8) | (b as u16)) as u16);
                    },
                    _ => panic!("???"),
                }
            }
        }

        match t {
            b'%' => VarValue::Integer(0),
            b'!' => VarValue::Single(0.0),
            b'#' => VarValue::Double(0.0),
            b'$' => VarValue::String(0, 0),
            _ => panic!("gdi"),
        }
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
