use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::path::Path;
use std::collections::HashMap;

use huffman::get_char_freqs;

fn print_map_value(mp: &HashMap<char, i32>, k: char) {
    match mp.get(&k) {
        Some(v) => println!("Value {} for key {}", v, k),
        _ => println!("Not found")
    };
}

fn main() {
    let args: Vec<String> = env::args().collect();
    
    let filepath = &args[1];
    println!("Reading the file at {}.", filepath);

    let path = Path::new(filepath);
    let display = path.display();

    // Open the path in read-only mode, returns `io::Result<File>`
    let mut file = match File::open(&path) {
        Err(why) => panic!("couldn't open {}: {}", display, why),
        Ok(file) => file,
    };

    // Read the file contents into a string, returns `io::Result<usize>`
    let mut text = String::new();
    match file.read_to_string(&mut text) {
        Err(why) => panic!("couldn't read {}: {}", display, why),
        Ok(size) => println!("Successfully read {}, {} bytes", display, size),
    }

    let freqs = get_char_freqs(&text);
    print_map_value(&freqs, 'X');
    print_map_value(&freqs, 't');
}
