while true; do
    ./mqpub -n 3
	echo "incident happen"
    ./mqpub -n 1
	echo "loop video happen"
    sleep 5
    ./mqpub -n 0
	echo "loop video close"
    sleep 10
    ./mqpub -q 16
	echo "enter defendant mode"
    sleep 3
    ./mqpub -q 17
	echo "defendant happen"
    sleep 15
done
 
