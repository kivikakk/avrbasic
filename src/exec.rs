#[cfg(target_arch = "avr")]
use arduino::{DDRB, PINB, PORTB};

#[cfg(target_arch = "avr")]
mod heaps {
    use synced::Synced;

    pub static VHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new(0x100 as *mut _)) };
    pub static SHEAP: Synced<Synced<*mut u8>> =
        unsafe { Synced::new(Synced::new(0x500 as *mut _)) };
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

    loop {
        add_var([b'A', 0], VarValue::Integer(105));
        let c = readch();
        let v = get_var([c, 0], b'%');

        match v {
            VarValue::Integer(i) => {
                for c in format_integer(i) {
                    putch(c);
                }
                putch(b'\n');
            }
            _ => (),
        }
    }
}

fn init() {
    #[cfg(target_arch = "avr")]
    unsafe {
        DDRB.write_volatile(0x01);
        VHEAP.write_bytes(0, 0x400);
    }
}

fn readch() -> u8 {
    #[cfg(target_arch = "avr")]
    unsafe { PINB.read_volatile() }

    #[cfg(not(target_arch = "avr"))]
    {
        use termios::{tcsetattr, Termios, ECHO, ICANON, TCSANOW};
        use std::io::{self, Read};

        let mut b: [u8; 1] = [0];
        let termios = Termios::from_fd(0).unwrap();
        let mut termios_read = termios.clone();
        termios_read.c_lflag &= !(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &mut termios_read).unwrap();
        io::stdin().read_exact(&mut b).unwrap();
        tcsetattr(0, TCSANOW, &termios).unwrap();
        b[0]
    }
}

fn putch(c: u8) {
    #[cfg(not(target_arch = "avr"))]
    {
        use std::io::{self, Write};

        let stdout = io::stdout();
        let mut stdout = stdout.lock();
        stdout.write(&[c]).unwrap();
        stdout.flush().unwrap();
    }
}

struct IntegerFormatter {
    i: i32,
    m: i32,
}

impl Iterator for IntegerFormatter {
    type Item = u8;

    fn next(&mut self) -> Option<u8> {
        if self.m == 0 {
            return None;
        }

        if self.i < 0 {
            self.i = -self.i;
            return Some(b'-');
        }

        if self.i >= self.m {
            let n = self.i / self.m;
            let r = b'0' + n as u8;
            self.i -= n * self.m;
            self.m /= 10;
            return Some(r);
        }

        self.m /= 10;

        Some(b'0')
    }
}

fn format_integer(i: i16) -> IntegerFormatter {
    let mut m = 10000;
    let ai = if i < 0 { -(i as i32) } else { i as i32 };
    while m > ai && m > 0 {
        m = m / 10;
    }
    if i == 0 {
        m = 1;
    }
    IntegerFormatter { i: i as i32, m: m }
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

    fn assert_int(i: i16) {
        assert_eq!(
            format_integer(i).map(|e| e as char).collect::<String>(),
            format!("{}", i)
        );
    }

    #[test]
    fn formatter() {
        for &i in &[
            0, -1, 1, 9, -9, 10, -10, 11, -11, 75, -75, 99, -99, 100, -100, 101, 777, -4258, 32767,
            -32768,
        ] {
            assert_int(i);
        }
    }
}
