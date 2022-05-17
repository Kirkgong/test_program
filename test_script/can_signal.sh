count=0
path="/pps/watermark"
# path="/home/nio/pps/watermark"

while true; do
    # echo $(($count % 4))

    # 转向灯切换
    if [ $(($count % 4)) = 1 ]; then
        echo LeTurnIndcrLiSts::1 >>$path
        echo RiTurnIndcrLiSts::1 >>$path
    fi

    if [ $(($count % 4)) = 3 ]; then
        echo LeTurnIndcrLiSts::0 >>$path
        echo RiTurnIndcrLiSts::0 >>$path
    fi

    # 挡位切换
    if [ $(($count % 40)) = 5 ]; then
        echo VCUActGear::0 >>$path
    fi

    if [ $(($count % 40)) = 10 ]; then
        echo VCUActGear::1 >>$path
    fi

    if [ $(($count % 40)) = 20 ]; then
        echo VCUActGear::2 >>$path
    fi

    if [ $(($count % 40)) = 30 ]; then
        echo VCUActGear::3 >>$path
    fi

    # 车速
    echo VCUVehDispSpd::$(($count % 300)) >>$path

    # 自动驻车
    if [ $(($count % 40)) = 3 ]; then
        echo AVHSts::0 >>$path
    fi
    if [ $(($count % 40)) = 13 ]; then
        echo AVHSts::1 >>$path
    fi
    if [ $(($count % 40)) = 23 ]; then
        echo AVHSts::2 >>$path
    fi
    if [ $(($count % 40)) = 33 ]; then
        echo AVHSts::3 >>$path
    fi

    # 电子驻车
    if [ $(($count % 37)) = 5 ]; then
        echo EPBSts::0 >>$path
    fi
    if [ $(($count % 37)) = 14 ]; then
        echo EPBSts::1 >>$path
    fi
    if [ $(($count % 37)) = 25 ]; then
        echo EPBSts::2 >>$path
    fi
    if [ $(($count % 37)) = 35 ]; then
        echo EPBSts::3 >>$path
    fi

    # 安全带
    if [ $(($count % 18)) = 2 ]; then
        echo SeatBltFrntSts::1 >>$path
    fi

    if [ $(($count % 18)) = 11 ]; then
        echo SeatBltFrntSts::0 >>$path
    fi

    # 刹车加速
    if [ $(($count % 8)) = 1 ]; then
        echo AccPedlSts::1 >>$path
        echo BrkPedlSts::0 >>$path
    fi

    if [ $(($count % 8)) = 5 ]; then
        echo AccPedlSts::0 >>$path
        echo BrkPedlSts::1 >>$path
    fi

    count=$(($count+1))  
    sleep 1
done
