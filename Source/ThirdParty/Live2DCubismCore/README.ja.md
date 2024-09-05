[English](README.md) / [日本語](README.ja.md)

---

# Live2D Cubism Core

このフォルダーには、ネイティブアプリケーションを開発するためのヘッダーおよびプラットフォーム固有のライブラリファイルが含まれています。

Cubism Coreは、Cubismモデルをアプリケーションにロードするための小さなライブラリです。


## はじめに

モデルを画面に表示して、アニメーションを再生するための情報については、[CubismNativeSamples](https://github.com/Live2D/CubismNativeSamples)をご覧ください。


## ファイル構造

```
Core
├─ dll          # Shared (Dynamic) Library files
├─ include      # Header file
└─ lib          # Static Library files
```


## ライブラリリスト

| プラットフォーム | アーキテクチャ | dll | lib | パス | 注記 |
| --- | --- | --- | --- | --- | --- |
| Windows | x86 | ✓ | ✓ | windows/x86 |   |
| Windows | x86_64 | ✓ | ✓ | windows/x86_64 |   |

### Windows用スタティックライブラリ

ライブラリは、VC++バージョン名ディレクトリの下にあります。

以下は、VC++バージョンのVisual Studioバージョンです。

| VC++バージョン | Visual Studioバージョン |
| ---: | --- |
| 120 | Visual Studio 2013 |
| 140 | Visual Studio 2015 |
| 141 | Visual Studio 2017 |
| 142 | Visual Studio 2019 |
| 143 | Visual Studio 2022 |

また、サフィックスライブラリ（`_MD`、`_MDd`、`_MT`、`_MTd`）は、ライブラリのコンパイルに使用される[ランタイムライブラリの使用](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library) オプションを参照します。

### 呼び出し規約

*Windows/x86*のダイナミックリンクライブラリを使用する場合は、呼び出し規約として明示的に`__stdcall`を使用してください。
