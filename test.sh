#!/bin/bash
echo "Tests start"

function compile(){
  echo "$1" | ./ccc > tmp.s

  #コンパイルチェック
  if [ $? -ne 0 ]; then
    echo "Failed to comile $1"
    exit 1 #fail
  fi

  #tmp.outという実行ファイルを作る driver.cとtmp.cを使って
  gcc -o tmp.out driver.c tmp.s || exit

  if [ $? -ne 0 ]; then
    echo "GCC failed"
    exit 1 #fail
  fi

}

function test(){
  expected="$1"
  exr="$2"
  echo "test $1 $2"
  compile "$exr"

  rlt="`./tmp.out`" #実行ファイルを実行して、標準出力を取得

  if [ "$expected" != "$rlt" ]; then
    echo "Test failed $expected extected but got $rlt .."
    exit 1 #fail
  else
    echo "OK"
  fi

}

make -s ccc

#test
test 0 0
test 42 42 

test 3 3 

test a '"a"'
test bbc '"bbc"' 

test 1+2 '"1+2"'

test 2 '1+1'
test 0 '1-1'
test 2 '0+2'

test 5 '1 + 4'
test 2 '3- 1'
test 8 '5 +3'

# rm -f tmp.out tmp.s
echo "All tests passed"
