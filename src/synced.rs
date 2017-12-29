// https://github.com/rust-lang/rust/issues/35035
#[cfg(target_arch = "avr")]
use core::{marker, ops};
#[cfg(not(target_arch = "avr"))]
use std::{marker, ops};

pub struct Synced<T>(T);

impl<T> Synced<T> {
    pub const unsafe fn new(inner: T) -> Synced<T> {
        Synced(inner)
    }
}

impl<T> ops::Deref for Synced<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

unsafe impl<T> marker::Sync for Synced<T> {}
