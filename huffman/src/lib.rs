use std::collections::HashMap;

pub fn get_char_freqs(s: &String) -> HashMap<char, i32> {
    let mut freqs = HashMap::new();
    for c in s.chars() { 
        match freqs.get(&c) {
            Some(freq) => freqs.insert(c, freq + 1),
            _ => freqs.insert(c, 1),
        };
    }
    return freqs;
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::File;
    use std::io::prelude::*;
    use std::path::Path;

    #[test]
    fn test_freqs() {
        let path = Path::new("./test.txt");
        let display = path.display();

        let mut file = match File::open(&path) {
            Err(why) => panic!("couldn't open {}: {}", display, why),
            Ok(file) => file,
        };

        let mut text = String::new();
        match file.read_to_string(&mut text) {
            Err(why) => panic!("couldn't read {}: {}", display, why),
            Ok(_) => (),
        }

        let freqs = get_char_freqs(&text);
        assert_eq!(freqs.get(&'X'), Some(&333));
        assert_eq!(freqs.get(&'t'), Some(&223000));
    }
}