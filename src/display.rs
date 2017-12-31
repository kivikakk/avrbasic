#[cfg(target_arch = "avr")]
use core::cell::Cell;
#[cfg(target_arch = "avr")]
use synced::Synced;
#[cfg(target_arch = "avr")]
use core::fmt;
#[cfg(not(target_arch = "avr"))]
use std::fmt;

#[cfg(target_arch = "avr")]
#[link(name = "u8g2")]
extern "C" {
    pub fn init_st7920();
    fn prep_display();
    // fn draw_str(x: u8, y: u8, str: *const i8);
    fn draw_strn(x: u8, y: u8, str: *mut i8, off: u8, n: u8);
    fn send_display();
}

#[cfg(target_arch = "avr")]
static IN_GETCH: Synced<Cell<Option<u8>>> = unsafe { Synced::new(Cell::new(None)) };

pub fn getch() -> u8 {
    #[cfg(target_arch = "avr")]
    unsafe {
        use arduino::{ADC, ADCSRA, ADSC};

        loop {
            ADCSRA.write_volatile(ADCSRA.read_volatile() | ADSC);
            while ADCSRA.read_volatile() & ADSC != 0 {}

            let ch = match ADC.read_volatile() {
                600...620 => b'A',
                690...710 => b'B',
                845...865 => b'C',
                920...940 => b'D',
                1005...1025 => b'E',
                _ => {
                    IN_GETCH.set(None);
                    continue;
                }
            };

            if IN_GETCH.get() != Some(ch) {
                IN_GETCH.set(Some(ch));
                return ch;
            }
        }
    }

    #[cfg(not(target_arch = "avr"))]
    {
        use termios::{tcsetattr, Termios, ECHO, ICANON, TCSANOW};
        use std::io::{self, Read};

        let mut b: [u8; 1] = [0];
        let termios = Termios::from_fd(0).unwrap();
        let mut termios_read = termios;
        termios_read.c_lflag &= !(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &termios_read).unwrap();
        io::stdin().read_exact(&mut b).unwrap();
        tcsetattr(0, TCSANOW, &termios).unwrap();
        b[0]
    }
}

pub fn getline() -> ([u8; 128], u8) {
    let mut l: [u8; 128] = [0; 128];
    let mut i = 0;

    loop {
        let c = getch();
        match c {
            b'A'...b'Z'
            | b'0'...b'9'
            | b' '
            | b'!'
            | b'#'
            | b'$'
            | b'%'
            | b'*'
            | b'+'
            | b'-'
            | b'='
            | b'/' => {
                if i < 128 {
                    putch(c);
                    flush();
                    l[i] = c;
                    i += 1;
                }
            }
            // for ease of testing
            b'a'...b'z' => {
                if i < 128 {
                    putch(c - (b'a' - b'A'));
                    flush();
                    l[i] = c - (b'a' - b'A');
                    i += 1;
                }
            }
            // backspace
            127 => {
                if i > 0 {
                    putstr(&[8, b' ', 8]);
                    flush();
                    i -= 1;
                }
            }
            10 => {
                putch(b'\n');
                flush();
                return (l, i as u8);
            }
            _ => (),
        }
    }
}

#[cfg(target_arch = "avr")]
mod avr_display {
    use super::{draw_strn, prep_display, send_display, Cell, Synced};

    const W: usize = 21;
    const H: usize = 6;
    static X: Synced<Cell<u8>> = unsafe { Synced::new(Cell::new(0)) };
    static Y: Synced<Cell<u8>> = unsafe { Synced::new(Cell::new(0)) };
    static LINES: Synced<Cell<[u8; W * H]>> = unsafe { Synced::new(Cell::new([0; W * H])) };

    pub fn putch(c: u8) {
        let mut lines = LINES.get();
        let x = X.get();
        let y = Y.get();

        let arr: *mut u8 = &mut lines as *const _ as *mut _;

        if c == 8 {
            if x > 0 {
                X.set(x - 1);
            }
        } else if c == 10 {
            if y < 5 {
                Y.set(y + 1);
                X.set(0);
            } else {
                unsafe {
                    for i in 0..(H - 1) {
                        arr.add(i as usize * W)
                            .copy_from_nonoverlapping(arr.add((i + 1) as usize * W), W);
                    }
                    arr.add((H - 1) * W).write_bytes(0, W);
                }
                X.set(0);
            }
        } else {
            unsafe {
                arr.add((y as usize * W) + x as usize).write(c);
            }
            X.set(x + 1);
        }

        LINES.set(lines);
    }

    pub fn draw() {
        unsafe {
            let mut lines = LINES.get();
            let arr: *mut u8 = &mut lines as *const _ as *mut _;

            prep_display();
            for y in 0..H {
                let offset = (y * W) as usize;
                draw_strn(0, y as u8, arr as *mut i8, offset as u8, W as u8);
            }
            send_display();
        }
    }

}

pub fn putch(c: u8) {
    #[cfg(target_arch = "avr")]
    avr_display::putch(c);

    #[cfg(not(target_arch = "avr"))]
    {
        use std::io::{self, Write};

        let stdout = io::stdout();
        let mut stdout = stdout.lock();
        stdout.write_all(&[c]).unwrap();
    }
}

pub fn flush() {
    #[cfg(target_arch = "avr")]
    avr_display::draw();

    #[cfg(not(target_arch = "avr"))]
    ::std::io::Write::flush(&mut ::std::io::stdout()).unwrap();
}

pub fn putstr(cs: &[u8]) {
    for &c in cs {
        putch(c);
    }
}

pub struct Screen;

impl fmt::Write for Screen {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        putstr(s.as_bytes());
        flush();
        Ok(())
    }
}

pub struct IntegerFormatter {
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

pub fn format_integer(i: i16) -> IntegerFormatter {
    let mut m = 10_000;
    let ai = if i < 0 { -i32::from(i) } else { i32::from(i) };
    while m > ai && m > 0 {
        m /= 10;
    }
    if i == 0 {
        m = 1;
    }
    IntegerFormatter {
        i: i32::from(i),
        m: m,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

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
