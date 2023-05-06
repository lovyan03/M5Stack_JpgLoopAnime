M5Stack JpgLoopAnime
===

M5Stack シリーズ (Basic, Gray, Core2, etc) 用 JPEG ループアニメーション

## Description
  
M5StackのSDカードで指定フォルダ内にあるJPEGをループ再生します  
描画はデュアルコア使用とDMA転送により高速に実行します。  
  
JpgLoopAnime.ino を開き、imageDirに画像ファイルのあるディレクトリ名を指定してビルドしてください。  
  
なおディレクトリ内の全ての画像をオンメモリしますので、容量にご注意ください。読込中にメモリが足りなくなった場合は読込みをスキップします。  
  

## 必要なライブラリ Requirement library

* https://github.com/m5stack/M5Unified/
* https://github.com/m5stack/M5GFX/
* https://github.com/tobozo/M5Stack-SD-Updater/  


動作確認は M5Unified 0.1.6 M5GFX 0.1.6 M5Stack-SD-Updater 1.2.5 にて行いました。


## 含まれているライブラリ Included library
[TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html) を改造して使用しています。  


## Licence

[MIT](https://github.com/lovyan03/M5Stack_JpgLoopAnime/blob/master/LICENSE)  

## Author

[lovyan03](https://twitter.com/lovyan03)  
