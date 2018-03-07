#!/bin/bash
EXEC="$@"

for f in $(find . -iname "*reduced*.aig" | sort --version-sort ;find . -iname "*.aig" | grep -v "reduced" | sort --version-sort); do
echo $f; 
exp=20
res=$(  abc -c "read ${f} ; dprove" 2>&1);
if [ "$?" -ne "0" ]; then
echo "ABC Failed to finish (exit code $found )"
exp=0
else
if (echo $res| grep -i 'Networks are not equivalent.' > /dev/null ); then
exp=10
elif  (echo $res| grep 'out of time'  > /dev/null ); then
exp=0
fi
fi
$EXEC $f &>/dev/null;found=$?;
if [ "$found" == "1" ]; then
if [ "$exp" == "10" ] ; then
echo "Error - failed to solve!"
exit 1
fi
echo "Expected $exp"
echo "Failed to finish"
else
echo "Expected $exp, found $found"
 
if [ $found -ne 10 ]; then
  if [ $found -ne 20 ]; then
    echo "Error on $f"
   exit
  fi
fi
if [ "$exp" -ne "0" ]; then
if [ $exp -ne $found ]; then
    echo "Error on $f: expected $exp, found $found"
   exit
fi
fi
#if [ $exp -ne $found ]; then 
#echo "error on $f: found ${found}, expected ${exp}"; break;  
#fi; 

fi
done
echo "Done Units"
failed=0
for i in {1..10000}; do
echo "running fuzz $i (timed out on ${failed} out of$i fuzz tests)"
aigfuzz -S -1 -m  > fuzz.aig

exp=20
if  abc -c "read fuzz.aig; dprove" | grep 'Networks are not equivalent.' >/dev/null; then
exp=10
fi

$EXEC fuzz.aig &>/dev/null;found=$?;
if [ "$found" == "1" ]; then
echo "Failed to finish"
let failed=failed+1
else
echo "Expected $exp, found $found"
 
if [ $found -ne 10 ]; then
  if [ $found -ne 20 ]; then
    echo "Error on fuzz.aig"
    cp -f fuzz.aig error.aig
   exit
  fi
fi

if [ $exp -ne $found ]; then
    echo "Error on fuzz.aig: expected $exp, found $found"
    cp -f fuzz.aig error.aig
   exit
fi

fi
done
echo "Timed out on $failed"
echo "Done All"
