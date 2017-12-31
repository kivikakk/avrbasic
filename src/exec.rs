use display::{flush, format_integer, getline, putch, putstr, Screen};
use parser::parse;

#[cfg(target_arch = "avr")]
use core::fmt::Write;
#[cfg(not(target_arch = "avr"))]
use std::fmt::Write;

#[cfg(target_arch = "avr")]
const VHEAP_BASE: u16 = 0x200;
const VHEAP_SIZE: u16 = 0x200;
const SHEAP_SIZE: u16 = 0x200;

#[cfg(target_arch = "avr")]
mod heaps {
    use synced::Synced;
    use super::{VHEAP_BASE, VHEAP_SIZE};

    pub static VHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new((VHEAP_BASE) as *mut _)) };
    pub static SHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new((VHEAP_BASE + VHEAP_SIZE) as *mut _)) };
}

#[cfg(not(target_arch = "avr"))]
mod heaps {
    use synced::Synced;
    use super::{SHEAP_SIZE, VHEAP_SIZE};

    static mut _VHEAP: [u8; VHEAP_SIZE as usize] = [0; VHEAP_SIZE as usize];
    lazy_static! {
        pub static ref VHEAP: Synced<*mut u8> = unsafe {
            Synced::new(&mut _VHEAP as *const _ as *mut _)
        };
    }

    static mut _SHEAP: [u8; SHEAP_SIZE as usize] = [0; SHEAP_SIZE as usize];
    lazy_static! {
        pub static ref SHEAP: Synced<*mut u8> = unsafe {
            Synced::new(&mut _SHEAP as *const _ as *mut _)
        };
    }
}

use self::heaps::{SHEAP, VHEAP};

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

    loop {
        let (s, l) = getline();

        match parse(&s[..l as usize]) {
            Ok(()) => (),
            Err(e) => {
                write!(&mut Screen, "{:?}", e).unwrap();
                putch(b'\n');
            }
        }
    }
}

fn init() {
    #[cfg(target_arch = "avr")]
    unsafe {
        use arduino::{ADPS0, ADPS1, REFS0, ADCSRA, ADEN, ADMUX};
        use display;

        VHEAP.write_bytes(0, VHEAP_SIZE as usize);
        SHEAP.write_bytes(0, SHEAP_SIZE as usize);

        ADMUX.write_volatile(ADMUX.read_volatile() | REFS0);
        ADCSRA.write_volatile(ADCSRA.read_volatile() | ADPS1 | ADPS0);
        ADCSRA.write_volatile(ADCSRA.read_volatile() | ADEN);

        display::init_st7920();
    }
}

pub fn add_var(vn: [u8; 2], val: &VarValue) {
    unsafe {
        let mut h = **VHEAP;

        loop {
            if h.read() != 0
                && (h.read() != vn[0] || h.add(1).read() != vn[1]
                    || h.add(2).read() != match *val {
                        VarValue::Integer(..) => 1,
                        VarValue::String(..) => 4,
                        _ => unreachable!(),
                    }) {
                let t = h.add(2).read();
                match t {
                    1 => h = h.add(5),
                    4 => h = h.add(6),
                    _ => unreachable!(),
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
                _ => unreachable!(),
            }

            break;
        }
    }
}

pub fn get_var(vn: [u8; 2], t: u8) -> VarValue {
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
                        _ => unreachable!(),
                    }
                }
            }

            match h.add(2).read() {
                1 => h = h.add(5),
                4 => h = h.add(6),
                _ => unreachable!(),
            }
        }

        match t {
            b'%' => VarValue::Integer(0),
            b'!' => VarValue::Single(0.0),
            b'#' => VarValue::Double(0.0),
            b'$' => VarValue::String(0, 0),
            _ => unreachable!(),
        }
    }
}

pub fn add_str(s: &[u8]) -> (u8, u16) {
    assert!(s.len() <= 255);
    let mut so: u16 = 0;

    unsafe {
        let mut h = **VHEAP;

        loop {
            if h.read() == 0 {
                break;
            }

            match h.add(2).read() {
                1 => h = h.add(5),
                4 => {
                    let off = h.add(3).read();
                    let b = h.add(4).read();
                    let a = h.add(5).read();
                    let len = (u16::from(a) << 8) | u16::from(b);
                    let latest: u16 = u16::from(off) + len;
                    if latest > so {
                        so = latest as u16;
                    }
                    h = h.add(6);
                }
                _ => unreachable!(),
            }
        }

        if so as usize + s.len() >= SHEAP_SIZE as usize {
            panic!("string heap too large");
        }

        SHEAP
            .add(so as usize)
            .copy_from_nonoverlapping(s as *const [u8] as *const u8, s.len());

        (s.len() as u8, so)
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

        let (len, o) = add_str(b"HELLO");
        assert_eq!(o, 0);
        assert_eq!(len, 5);
        add_var([b'A', 0], &VarValue::String(len, o));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(len, o));

        let (len, o) = add_str(b"HI");
        assert_eq!(o, 5);
        assert_eq!(len, 2);
        add_var([b'A', 0], &VarValue::String(len, o));
        assert_eq!(get_var([b'A', 0], b'%'), VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'$'), VarValue::String(len, o));
    }
}
