use display::{flush, getline, putch, putstr, Screen};
use interp::interp;

#[cfg(target_arch = "avr")]
use core::fmt::Write;
#[cfg(not(target_arch = "avr"))]
use std::fmt::Write;

#[cfg(target_arch = "avr")]
use core::slice;
#[cfg(not(target_arch = "avr"))]
use std::slice;

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
    Integer(i16),        // %
    Single(f32),         // !
    Double(f64),         // #
    String(BoxedString), // $
}

pub type Var = ([u8; 2], VarValue);

pub fn run() {
    init();

    putstr(b"AVR-BASIC\n");
    flush();

    loop {
        let (s, l) = getline();

        match interp(&s[..l as usize]) {
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
                    4 => h = h.add(5),
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
                VarValue::String(ref bs) => {
                    h.add(2).write(4);
                    h.add(3).write(bs.sheap_offset as u8);
                    h.add(4).write(((bs.sheap_offset >> 8) & 0xFF) as u8);
                }
                _ => unreachable!(),
            }

            break;
        }
    }
}

pub fn get_var(vn: [u8; 2], t: u8) -> Option<VarValue> {
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
                            return Some(VarValue::Integer(
                                (u16::from(a) << 8 | u16::from(b)) as i16,
                            ));
                        },
                        4 => if t == b'$' {
                            let b = h.add(3).read();
                            let a = h.add(4).read();
                            return Some(VarValue::String(BoxedString::from_offset(
                                ((u16::from(a) << 8) | u16::from(b)),
                            )));
                        },
                        _ => unreachable!(),
                    }
                }
            }

            match h.add(2).read() {
                1 => h = h.add(5),
                4 => h = h.add(5),
                _ => unreachable!(),
            }
        }

        None
    }
}

pub fn clear_vars() {
    unsafe {
        VHEAP.write(0);
        SHEAP.write(0);
    }
}

#[derive(Debug, PartialEq)]
pub struct BoxedString {
    len: u8,
    sheap_offset: u16,
}

impl BoxedString {
    pub fn new(s: &[u8]) -> BoxedString {
        Self::from_multiple(&[s])
    }

    pub fn from_multiple(ss: &[&[u8]]) -> BoxedString {
        let len: usize = ss.iter().map(|s| s.len()).sum();
        if len > 255 {
            panic!("string too large");
        }

        let len = len as u8;

        unsafe {
            let mut sheap = **SHEAP;

            loop {
                if SHEAP.offset_to(sheap).unwrap() >= SHEAP_SIZE as isize {
                    panic!("string heap too large");
                }

                let b = sheap.read();
                if b > 0 {
                    sheap = sheap.add(b as usize + 1);
                    continue;
                }

                if SHEAP.offset_to(sheap.add(len as usize + 1)).unwrap() > SHEAP_SIZE as isize {
                    panic!("string heap too large");
                }

                sheap.write(len as u8);
                let mut off = 1;
                for s in ss {
                    sheap
                        .add(off)
                        .copy_from_nonoverlapping(*s as *const [u8] as *const u8, s.len());
                    off += s.len();
                }

                return BoxedString {
                    len: len,
                    sheap_offset: SHEAP.offset_to(sheap).unwrap() as u16,
                };
            }
        }
    }

    pub fn from_offset(sheap_offset: u16) -> BoxedString {
        BoxedString {
            len: unsafe { SHEAP.add(sheap_offset as usize).read() },
            sheap_offset: sheap_offset,
        }
    }

    pub fn value(&self) -> &'static [u8] {
        unsafe {
            slice::from_raw_parts(SHEAP.add(self.sheap_offset as usize + 1), self.len as usize)
        }
    }

    pub fn into_varvalue(self) -> VarValue {
        VarValue::String(self)
    }
}

impl Clone for BoxedString {
    fn clone(&self) -> BoxedString {
        BoxedString::new(self.value())
    }
}

impl Drop for BoxedString {
    fn drop(&mut self) {
        // TODO:
        // defragment heap? (what about BoxedStrings not in VHEAP/only on stack?)
        // mark space as unused? (will be fragmented)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add() {
        clear_vars();

        add_var([b'A', 0], &VarValue::Integer(105));
        assert_eq!(get_var([b'A', 0], b'%'), Some(VarValue::Integer(105)));
        assert_eq!(get_var([b'A', 0], b'$'), None);

        let s = BoxedString::new(b"HELLO");
        let len = s.len;
        let sheap_offset = s.sheap_offset;
        assert_eq!(s.value(), b"HELLO");
        add_var([b'A', 0], &s.into_varvalue());
        assert_eq!(get_var([b'A', 0], b'%'), Some(VarValue::Integer(105)));
        match get_var([b'A', 0], b'$') {
            Some(VarValue::String(ref bs)) => {
                assert_eq!(bs.len, len);
                assert_eq!(bs.sheap_offset, sheap_offset);
            }
            _ => panic!("no string"),
        }

        let s = BoxedString::new(b"HI");
        let len = s.len;
        let sheap_offset = s.sheap_offset;
        assert_eq!(s.value(), b"HI");
        add_var([b'A', 0], &s.into_varvalue());
        assert_eq!(get_var([b'A', 0], b'%'), Some(VarValue::Integer(105)));
        let v = get_var([b'A', 0], b'$');
        match v {
            Some(VarValue::String(ref bs)) => {
                assert_eq!(bs.len, len);
                assert_eq!(bs.sheap_offset, sheap_offset);
            }
            _ => panic!("no string"),
        }
    }
}
