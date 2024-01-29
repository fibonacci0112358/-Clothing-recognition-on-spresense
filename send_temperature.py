import serial
import time
import requests
from datetime import datetime

city_code = "270000" # 大阪のcityコード
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
        return str((int(max_temp) + int(min_temp)) / 2)

def main():
    port = "COM5" # ポート番号
    baudrate = 115200  # ボーレート
    a = 1
    try:
        # シリアルポートを開く
        ser = serial.Serial(port, baudrate,timeout=0.8) # timeoutは受信したときの待ち時間
        
        # 何かしらでループを終了させる処理が必要!!!!!!!
        while True:
            # a += 1
            # b = str(a).encode('ascii')
            # ser.write(b)
            # print(f"send: {a}")
            # time.sleep(1)
            # data = ser.readline().decode().strip() # 1秒間で受信した文字列をすべて読み込む
            # print(f"Received: {data}")
            
            # シリアルの受信を待つ場合 
            # 加えてspresense側で認識結果をスタックしてより正確な認識結果を取得する
            if ser.in_waiting > 0: # 受信バッファにデータがある場合
                data = ser.readline().decode().strip() # 受信した文字列をすべて読み込む
                print(f"Received: {data}")
                ser.write(get_temperature().encode('ascii'))
                time.sleep(5)

    except serial.SerialException as e:
        print(f"Serial Exception: {e}")

    finally:
        # シリアルポートを閉じる
        if ser.is_open:
            ser.close()
            print("Serial port closed")


if __name__ == "__main__":
    main()
