count=1
while true;
do
mkdir -p /fota/slog_$count 
cp -r /blackbox/cdclogs/qnx/slog/  /fota/slog_$count 
sleep 3600
((count++))
done
