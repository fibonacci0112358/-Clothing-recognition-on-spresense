import serial
import time
import requests
from datetime import datetime

city_code = "130010" #稚内のcityコード:011000 東京のcityコード:130010 那覇のcityコード:471010
url = "https://weather.tsukumijima.net/api/forecast/city/" + city_code

def get_temperature():
    try:
        response = requests.get(url)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print("Error:{}".format(e))
        return -100

    else:
        weather_json = response.json()
        # 2番目の要素 0:今日 1:明日 2:明後日
        max_temp = weather_json['forecasts'][1]['temperature']['max']['celsius']
        # 今日の最低気温は取得できないので、明日の最低気温を取得
        min_temp = weather_json['forecasts'][1]['temperature']['min']['celsius']
        print("最高気温 {} 最低気温 {}".format(max_temp, min_temp))
        return str((int(max_temp) + int(min_temp)) // 2) # 平均気温を整数値の文字列で返す

def main():
    port = "COM5" # ポート番号
    baudrate = 115200  # ボーレート
    a = 1
    try:
        # シリアルポートを開く
        ser = serial.Serial(port, baudrate,timeout=0.8) # timeoutは受信したときの待ち時間
        
        while True:
            # シリアルを受信するとAPIから気温を取得し、シリアル送信
            if ser.in_waiting > 0: # 受信バッファにデータがある場合
                data = ser.readline().decode().strip()
                print(f"Received: {data}") # 1:T-shirt 2:PullOver 3:Coat
                ser.write(get_temperature().encode('ascii'))

    except serial.SerialException as e:
        print(f"Serial Exception: {e}")

    finally:
        # シリアルポートを閉じる
        if ser.is_open:
            ser.close()
            print("Serial port closed")


if __name__ == "__main__":
    main()
