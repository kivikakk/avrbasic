#![feature(lang_items, unwind_attributes)]
#![feature(const_fn)]
#![no_std]
#![cfg_attr(target_arch = "avr", no_main)]

/*
extern crate arduino;
use arduino::{DDRB, PORTB};
use core::ptr::write_volatile;
unsafe { write_volatile(DDRB, 0xFF); }
 */

pub mod parser;
pub mod synced;
pub mod exec;

#[no_mangle]
#[cfg(target_arch = "avr")]
pub extern "C" fn main() {
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
