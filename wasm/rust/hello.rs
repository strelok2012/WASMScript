#![no_std]
#![feature(lang_items)]
#![feature(start)]
#![feature(libc)]

#![feature(fundamental)]

extern crate libc;

#[lang = "eh_personality"] extern fn eh_personality() {}
#[lang = "panic_fmt"] #[no_mangle] fn panic_fmt() -> ! { loop {} }




#[no_mangle]
pub fn exportFunction(i_test: isize) -> isize {
    let result = i_test+1;
    result
}

// Entry point for this program
#[start]
fn start(_argc: isize, _argv: *const *const u8) -> isize {
    0
}



