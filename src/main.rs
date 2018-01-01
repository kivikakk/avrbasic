#![feature(lang_items, unwind_attributes, pointer_methods, const_fn, const_cell_new, offset_to)]
#![cfg_attr(target_arch = "avr", no_std)]
#![cfg_attr(target_arch = "avr", no_main)]

extern crate arduino;
#[cfg(not(target_arch = "avr"))]
#[macro_use]
extern crate lazy_static;
#[cfg(not(target_arch = "avr"))]
extern crate termios;

pub mod interp;
pub mod synced;
pub mod exec;
pub mod display;

#[no_mangle]
#[cfg(target_arch = "avr")]
pub extern "C" fn main() {
    exec::run();
}

#[cfg(not(target_arch = "avr"))]
fn main() {
    exec::run();
}

#[cfg(target_arch = "avr")]
pub mod std {
    #[lang = "eh_personality"]
    #[no_mangle]
    pub unsafe extern "C" fn rust_eh_personality(
        _state: (),
        _exception_object: *mut (),
        _context: *mut (),
    ) -> () {
    }

    #[lang = "panic_fmt"]
    #[no_mangle]
    pub extern "C" fn rust_begin_panic(_msg: (), _file: &'static str, _line: u32) -> ! {
        loop {}
    }
}
