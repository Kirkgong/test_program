while true;
do
echo tt1::1 >> /pps/telltale
echo tt2::1 >> /pps/telltale
sleep 2
echo tt1::0 >> /pps/telltale
echo tt2::0 >> /pps/telltale
sleep 2
done
