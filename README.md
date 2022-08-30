InstaChordをBLE MIDIでESP32系のマイコンに接続して、コードのノート情報を受け取るarduinoコード

起動時に接続を試みます。途中で切れた場合はマイコンをリセットしてください。

# 参考（というよりほとんどそのままです）：
https://chakoku.hatenablog.com/entry/2020/08/22/235109
こちらはKORG MicroKeyで接続しています。 

bleClient->connect(pAddress);   
でMACアドレス指定だとなぜかInstaChordは接続できないので直接デバイスで接続しています。
