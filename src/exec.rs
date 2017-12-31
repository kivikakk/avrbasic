use display::{flush, format_integer, getline, putch, putstr};

#[cfg(target_arch = "avr")]
mod heaps {
    use synced::Synced;

    pub static VHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new(0x400 as *mut _)) };
    //    pub static SHEAP: Synced<Synced<*mut u8>> =
    //        unsafe { Synced::new(Synced::new(0x600 as *mut _)) };
}

#[cfg(not(target_arch = "avr"))]
mod heaps {
    use synced::Synced;

    static mut _VHEAP: [u8; 0x200] = [0; 0x200];
    lazy_static! {
        pub static ref VHEAP: Synced<*mut u8> = unsafe {
            Synced::new(&mut _VHEAP as *const _ as *mut _)
        };
    }
}

use self::heaps::VHEAP;

#[derive(Debug, PartialEq)]
pub enum VarValue {
    Integer(i16),    // %
    Single(f32),     // !
    Double(f64),     // #
    String(u8, u16), // $
}

pub type Var = ([u8; 2], VarValue);

pub fn run() {
    init();

    putstr(b"AVR-BASIC\n");
    flush();

    add_var([b'A', 0], &VarValue::Integer(105));

    loop {
        let s = getline();

        let v = get_var([s[0], 0], b'%');

        match v {
            VarValue::Integer(i) => {
                putstr(b"value: ");
                for c in format_integer(i) {
                    putch(c);
                }
                putch(b'\n');
            }
            VarValue::String(_o, _l) => {
                // ...
            }
            _ => (),
        }
    }
}

fn init() {
    #[cfg(target_arch = "avr")]
    unsafe {
        use arduino::{ADPS0, ADPS1, REFS0, ADCSRA, ADEN, ADMUX};
        use display;

        VHEAP.write_bytes(0, 0x200);

        ADMUX.write_volatile(ADMUX.read_volatile() | REFS0);
        ADCSRA.write_volatile(ADCSRA.read_volatile() | ADPS1 | ADPS0);
        ADCSRA.write_volatile(ADCSRA.read_volatile() | ADEN);

        display::init_st7920();
    }
}

fn add_var(vn: [u8; 2], val: &VarValue) {
    unsafe {
        let mut h = **VHEAP;

        loop {
            if h.read() != 0
                && (h.read() != vn[0] || h.add(1).read() != vn[1]
                    || h.add(2).read() != match *val {
                        VarValue::Integer(..) => 1,
                        VarValue::String(..) => 4,
                        _ => panic!("eh"),
                    }) {
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
            match *val {
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
        let mut h = **VHEAP;

        loop {
            let a1 = h.read();
            if a1 == 0 {
                break;
            } else if a1 == vn[0] {
                let a2 = h.add(1).read();
                if a2 == vn[1] {
                    match h.add(2).read() {
                        1 => if t == b'%' {
                            let b = h.add(3).read();
                            let a = h.add(4).read();
                            return VarValue::Integer((u16::from(a) << 8 | u16::from(b)) as i16);
                        },
                        4 => if t == b'$' {
                            let off = h.add(3).read();
                            let b = h.add(4).read();
                            let a = h.add(5).read();
                            return VarValue::String(off, ((u16::from(a) << 8) | u16::from(b)));
                        },
                        _ => panic!("???"),
                    }
                }
            }

            match h.add(2).read() {
                1 => h = h.add(5),
                4 => h = h.add(6),
                _ => panic!("eh"),
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
        add_var([b'A', 0], &VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(0, 0));

        add_var([b'A', 0], &VarValue::String(1, 1));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(1, 1));

        add_var([b'A', 0], &VarValue::String(2, 2));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(2, 2));
    }
}
