#include <Camera.h>
#include <DNNRT.h>
#include <SDHCI.h>
#include <Adafruit_ILI9341.h>
#include <BmpImage.h>

#define TFT_DC  9
#define TFT_CS  10
#define OFFSET_X  (48)
#define OFFSET_Y  (0)
#define CLIP_WIDTH (224)
#define CLIP_HEIGHT (224)
#define DNN_WIDTH  (28)
#define DNN_HEIGHT  (28)
#define N 4

SDClass SD;
DNNRT dnnrt;
BmpImage bmp;
DNNVariable input(DNN_WIDTH*DNN_HEIGHT);
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);
const String label[N] = {"None","T-shirt","Pullover","Coat"};
// ディスプレイに書き込むテキスト
String gStrResult;
int filling_color = ILI9341_BLACK;
// 認識結果が連続で同じ場合にカウントする
int same_count = 0;
// 前回のクラスを保存する変数
int prev_class = -1;
int temperature;
// 1:T-shirtと2:Pulloverの境界値
const int threshold1_2 = 20;
// 2:Pulloverと3:Coatの境界値
const int threshold2_3 = 10;

// cameraのstreamingのコールバック関数
void CamCB(CamImage img) {
  if (!img.isAvailable()) {
    Serial.println("Image is not available. Try again");
    return;
  }
  // カメラ画像の切り抜きと縮小
  CamImage small;
  CamErr err = img.clipAndResizeImageByHW(small
                     , OFFSET_X, OFFSET_Y
                     , OFFSET_X + CLIP_WIDTH -1
                     , OFFSET_Y + CLIP_HEIGHT -1
                     , DNN_WIDTH, DNN_HEIGHT);

  // 認識用モノクロ画像を設定
  uint16_t* imgbuf = (uint16_t*)small.getImgBuff();
  float *dnnbuf = input.data();
  for (int n = 0; n < DNN_HEIGHT*DNN_WIDTH; ++n) {
    dnnbuf[n] = (float)(((imgbuf[n] & 0xf000) >> 8) 
                      | ((imgbuf[n] & 0x00f0) >> 4))/255.;
  }
  // 推論の実行
  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  // 認識結果
  DNNVariable output = dnnrt.outputVariable(0);
  //　確率最大のクラス
  int cloth = output.maxIndex();
  gStrResult = "Detecting : " + label[cloth];
  
  // 認識結果が連続で同じ場合にシリアル通信で送信（誤検出防止）
  if(cloth == prev_class && cloth != 0){
    same_count++;
  }else{
    same_count = 0;
  }
  if(same_count > 20){
    // pythonにserial送信
    Serial.println(cloth);
    same_count = 0;
    // python側の送信処理が終わるまで待つ
    delay(2000);
    //気温の呼び出し
    temperature = Serial.readString().toInt();
    //結果の表示
    if(cloth==1){
      if(temperature >= threshold1_2){
        filling_color = ILI9341_GREEN;
      }
      else{
        filling_color = ILI9341_BLUE;
      }
    }
    else if(cloth==2){
      if(temperature >= threshold1_2){
        filling_color = ILI9341_RED;
      }
      else if(temperature >= threshold2_3){
        filling_color = ILI9341_GREEN;
      }
      else{
        filling_color = ILI9341_BLUE;
      }
    
    }
    else if(cloth==3){
      if(temperature >= threshold2_3){
        filling_color = ILI9341_RED;
      }
      else{
        filling_color = ILI9341_GREEN;
      }
    }
    else{
    }
  }
    prev_class = cloth;
    
  img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
  uint16_t* imgBuf = (uint16_t*)img.getImgBuff();
  drawBox(imgBuf, OFFSET_X, OFFSET_Y, CLIP_WIDTH, CLIP_HEIGHT, 5, filling_color); 
  display.drawRGBBitmap(0, 0, (uint16_t*)imgBuf, 320, 224);
  putStringOnLcd(gStrResult, ILI9341_YELLOW);
}

void setup() {

  Serial.begin(115200);
  while (!Serial);
  // SDカードの挿入待ち
  while (!SD.begin()) { 
    Serial.println("Insert SD");
  }
  
  int ret = 0;
  // SDカードにある学習済モデルの読み込み
  File nnbfile = SD.open("model.nnb");
  ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    return;
  }
  display.begin();
  display.setRotation(1);
  theCamera.begin();
  theCamera.startStreaming(true, CamCB);
}

void loop() { }
