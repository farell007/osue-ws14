#/bin/sh

echo ---------------------------
echo Welcome to OS test!
echo ---------------------------

i=1

for f in tests/*.test
do
    cat $f
    if command cat $f | diff $f.out - >/dev/null ; then
      echo Check ${i} passed
    else
      echo Check ${i} failed
    fi
    $i = $i + 1
    echo ---------------------------
done

