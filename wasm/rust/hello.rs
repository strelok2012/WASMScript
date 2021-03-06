#![no_std]
#![no_main]
#![feature(lang_items)]

#[lang = "panic_fmt"]  fn panic_fmt() -> ! { loop {} }

mod wasm {
    pub fn _import_function(i: isize) -> isize {
        unsafe { import_function(i) }
    }

    extern {
        fn import_function(i: isize) -> isize;
    }
}

#[no_mangle]
pub fn export_function(i_test: isize) -> isize {
	wasm::_import_function(i_test*2);
	let result = i_test+1;
	result
}

#[no_mangle]
pub extern fn main() { }




