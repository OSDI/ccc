#!/bin/bash
echo "Tests start"

function compile(){
  echo "$1" | ./ccc > tmp.s

  #コンパイルチェック
  if [ $? -ne 0 ]; then
    echo "Failed to comile $1"
    result=1
    return
  fi

  #tmp.outという実行ファイルを作る driver.cとtmp.cを使って
  gcc -o tmp.out driver.c tmp.s || exit

  if [ $? -ne 0 ]; then
    echo "GCC failed"
    result=1
    return
  fi

}

function fail_test(){
  expected="$1"
  exr="$2"
  echo "fail_test $1 $2"
  result=0
  compile "$exr"

  rlt="`./tmp.out`" #実行ファイルを実行して、標準出力を取得

  if [ "$result" != 0 ]; then
    echo "OK"
  else
    echo "Test passed but fail is extected .."
    exit 1 #fail
  fi

}

function assert_equal {
  if [ "$1" != "$2" ]; then
    echo "Test failed $2 extected but got $1 .."
    exit 1 #fail
  else:
    echo "OK"
  fi
}

function test_ast {
  echo "test ast $1 $2"
  result="$(echo "$2" | ./ccc -a)"
  if [ $? -ne 0 ]; then
    echo "Failed to comile $1"
    exit 1
  fi
  assert_equal "$result" "$1"
}


function test(){
  exr="$2"
  echo "test $1 $2"
  compile "$2"
  assert_equal "$(./tmp.out)" "$1"
}


make -s ccc

#test

test_ast '1' '1'
test_ast '100' '100'
test_ast '(+ 1 2)' '1+2'
test_ast '(+ (+ 1 2) 3)' '1+2+3'
test_ast '(+ (- (+ 1 2) 3) 4)' '1+2-3+4'
test_ast 'neko' '"neko"'

test 0 0
test 42 42 

test 3 3 

test a '"a"'
test bbc '"bbc"' 

test 1+2 '"1+2"'

test 2 '1+1'
test 0 '1-1'
test 2 '0+2'
test 10 '1+2+3+4'
test 4 '1+2-3+4'

test 5 '1 + 4'
test 8 '5 +3'

test 2 '2*1'
test 6 '2+2*2'
test 14 '1*2+3*4'
test 13 '5*2+3'
test 7 '2*3+1'

fail_test '"abc'
fail_test '0abc'
fail_test '1+'

# rm -f tmp.out tmp.s
echo "=================="
echo "All tests passed"
echo "=================="
