extern crate rand;

use std::convert::TryFrom;
use std::convert::TryInto;
use std::vec;

use super::*;

fn decode_block(indata: &[u8]) -> Vec<u8> {
    println!("dec size = {}", indata.len());

    if indata.len() == 0 {
        return vec![];
    }
    unsafe {
      let inlen = tb64declen(indata.as_ptr() as *mut ::std::os::raw::c_uchar, indata.len() );
      let mut decoded_data = vec![0u8; inlen ];
      let _ = tb64sdec(indata.as_ptr() as *mut ::std::os::raw::c_uchar, indata.len(), decoded_data.as_mut_ptr() as *mut u8);
      return decoded_data;
    }
}

fn encode_block(indata: &[u8]) -> Vec<u8> {
    println!("enc size = {}", indata.len());

    unsafe {
      let outlen = tb64enclen( indata.len() );
      let mut encoded_indata = vec![0u8; outlen];
      let _size = tb64senc(indata.as_ptr() as *mut u8, indata.len(), encoded_indata.as_mut_ptr() as *mut ::std::os::raw::c_uchar);
      return encoded_indata;
    }
}

fn test_block(inlen: usize) {
    let mut source = Vec::with_capacity(inlen);
    for i in 0..inlen {
        if rand::random() {
            source.push(u8::try_from(i+1).unwrap());
        }
    }

    if source.is_empty() {
        source = vec![1u8, 2]
    }

    let encoded = encode_block(&source[..]);
    let decoded = decode_block(&encoded[..]);
    assert_eq!(&source[..], &decoded[..]);
}

#[test]
fn sample_tb64() {
    let block_sizes = vec![1u32, 100, 3, 4, 8, 10, 16, 8, 32, 5, 64, 3, 128, 255];

    for _ in 1..10 {
        for size in &block_sizes[..] {
            println!("testing with cap = {}", size+0);
            test_block((size+0).try_into().unwrap());
        }
    }
}
