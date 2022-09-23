while true; do
    ./mqpub -n 1
	echo "loop video happen"
    sleep 5
    ./mqpub -n 0
	echo "loop video close"
    ./mqpub -q 16
	echo "enter defendant mode"
    ./mqpub -q 17
	echo "defendant happen"
    ./mqpub -q 18
	echo "defendant happen"
    ./mqpub -q 19
	echo "defendant happen"
    sleep 5
    ./mqpub -q 21
	echo "exit defendant"
done
 
