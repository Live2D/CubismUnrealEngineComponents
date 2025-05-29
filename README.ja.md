[English](README.md) / [日本語](README.ja.md)

---

# Cubism Unreal Engine Components

Unreal Engine用のCubism SDKのオープンコンポーネントです。

モデルをロードするには Live2D Cubism Core と組み合わせて使用します。

SDKパッケージのダウンロードページをお探しの場合は、[ダウンロードページ](https://www.live2d.com/sdk/download/unrealengine/)にアクセスしてください。

## ライセンス

ご使用前に[ライセンス](LICENSE.md)をお読みください。

## お知らせ

ご使用前に[お知らせ](NOTICE.ja.md)をお読みください。


## Cubism 5新機能や過去バージョンとの互換性について

本 SDK はCubism 5に対応した製品です。  
Cubism 5 Editorに搭載された新機能のSDK対応については [こちら](https://docs.live2d.com/cubism-sdk-manual/cubism-5-new-functions/)をご確認ください。  
過去バージョンのCubism SDKとの互換性については [こちら](https://docs.live2d.com/cubism-sdk-manual/compatibility-with-cubism-5/)をご確認ください。

## 構造

### コンポーネント

コンポーネントは役割ごとにグループ化されており、このグループ化はフォルダー構造と名前空間の両方に反映されます。

#### フレームワーク

このグループのコンポーネントとクラスは、リップシンクやCubismの組み込み用ファイルとUnreal Engineの統合などの追加機能を提供します。すべてのフレームワークコードは`./Source/Live2DCubismFramework`にあります。

#### レンダリング

このグループのコンポーネントとクラスは、Unreal Engineの機能を使用してCubismモデルをレンダリングする機能を提供します。コードは、`./Source/Live2DCubismFramework/Public/Rendering`及び`./Source/Live2DCubismFramework/Private/Rendering`にあります。

### エディター拡張機能

Unreal Engine Editor拡張機能は、`./Source/Live2DCubismFrameworkEditor`にあります。

### インポーター機能

Cubismファイルをuassetファイルに変換する機能は `./Source/Live2DCubismFrameworkImporter`にあります。

### リソース

シェーダー等のアセットのリソースは、`./Content/Materials`にあります。

## 開発環境

| Unreal Engine | バージョン |
| --- | --- |
| Unreal Editor 5.5 | 5.5.4 |
| Unreal Editor 5.4 | 5.4.4 |
| Unreal Editor 5.3 | 5.3.2 |

| ライブラリ / ツール | バージョン |
| --- | --- |
| Visual Studio 2022 | 17.14.1 |
| Windows SDK | 10.0.22621.0 |

## テスト済みの環境

| プラットフォーム | バージョン |
| --- | --- |
| Windows 11 | 24H2 |


## ブランチ

最新の機能や修正をお探しの場合、`develop`ブランチをご確認ください。

`master`ブランチは、公式のSDKリリースごとに`develop`ブランチと同期されます。

## 使用法

Unreal プロジェクトをC++で作成し、Unreal プロジェクトの下に`Plugins`フォルダを作成した後、パッケージフォルダごと作成した`Plugins`フォルダーにコピーしてUnreal Engineを起動し直してください。
詳しくは[公式マニュアル](https://docs.live2d.com/cubism-sdk-manual/cubism-sdk-for-unreal-engine/)をご確認ください。

## プロジェクトへの貢献

プロジェクトに貢献する方法はたくさんあります。バグのログの記録、このGitHubでのプルリクエストの送信、Live2Dコミュニティでの問題の報告と提案の作成です。

### フォークとプルリクエスト

修正、改善、さらには新機能をもたらすかどうかにかかわらず、プルリクエストに感謝します。メインリポジトリを可能な限りクリーンに保つために、必要に応じて個人用フォークと機能ブランチを作成してください。

### バグ

Live2Dコミュニティでは、問題のレポートと機能リクエストを定期的にチェックしています。バグレポートを提出する前に、Live2Dコミュニティで検索して、問題のレポートまたは機能リクエストがすでに投稿されているかどうかを確認してください。問題がすでに存在する場合は、関連するコメントを追記してください。

### 提案

SDKの将来についてのフィードバックにも関心があります。Live2Dコミュニティで提案や機能のリクエストを送信できます。このプロセスをより効果的にするために、それらをより明確に定義するのに役立つより多くの情報を含めるようお願いしています。

## コーディングガイドライン

Unreal Engine公式の[Unreal Engine における Epic の C++ コーディング規約](https://dev.epicgames.com/documentation/ja-jp/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine) に則っています。

## フォーラム

ご不明な点がございましたら、公式のLive2Dフォーラムに参加して、他のユーザーと話し合ってください。

- [Live2D 公式クリエイターズフォーラム](https://creatorsforum.live2d.com/)
- [Live2D Creators Forum(English)](https://community.live2d.com/)
