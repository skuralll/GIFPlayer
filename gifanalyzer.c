#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>

/*
参考文献
構造体について: https://9cguide.appspot.com/16-02.html
カレントディレクトリ内のファイルを一覧表示: https://torisky.com/
GIFフォーマットの詳細: http://www.tohoho-web.com/wwwgif.htm
GIFフォーマットの取得プログラム例(画像のWidth,Height取得処理部分のみを参考にした): https://reerishun.com/makerblog/?p=693
*/

/*
Block List
0x2c        Image Block
0x21 0xf9   Graphic Control Extension
0x21 0xfe   Comment Extension
0x21 0x01   Plain Text Extension
0x21 0xff   Application Extension

0x21 は拡張ブロックであることを示す

0x3b Trailer 終了 (1Byte) Blockではない
*/

const int MAX_FRAME = 512;//最大フレーム数

typedef struct{//rgb型を定義
    unsigned char r, g, b;
} rgb;

typedef struct {//gif_header型を定義
    char signature[4];//GIF
    char version[4];//バージョン
    unsigned int width;//画像幅
    unsigned int height;//画像高さ
    int gctf, cr, sf, sgct;//各種フラグなど
    int bgcolor;//背景色のインデックス
    int paratio;//ピクセルの縦横比
    rgb *gctable;//rgb gctable[256]; //グローバルカラーテーブル (gctfが1の場合に存在)
} gif_header;

typedef struct{//app_extension型を定義 Application Extension
    unsigned char app_indentifier[9];//Application Identifier(8 Bytes) + null = 9Byte "NETSCAPE"
    unsigned char app_authcode[4];//Application Authentication Code(3 Bytes) + null = 4Byte "2.0"
    unsigned char *app_data_sizes;//アプリケーションデータのサイズリスト
    unsigned char *app_data;//アプリケーションデータ郡。 1バイトの 0x01 および2バイトのループ回数(0～65535) 
} app_extension;

typedef struct{//gc_extension型を定義  Graphic Control Extension
    unsigned short delay_time;//Delay Time(2 Bytes)
} gc_extension;

typedef struct{//image型を定義  Image Block
    unsigned int pos_left; //GIF画像全体に対するこのイメージブロックの左端相対位置
    unsigned int pos_top; //GIF画像全体に対するこのイメージブロックの上端相対位置
    unsigned int width;//イメージブロック幅
    unsigned int height;//イメージブロック高さ
    int lct_flag, i_flag, sort_flag, slct;//各種フラグ 
    rgb *lctable;//rgb lctable[256];//ローカルカラーテーブル (lct_flagが1の場合に存在)
    unsigned char lzw_min_bit;//LZWコードの最小ビット数。
    //unsigned char image_data[256][256];//イメージデータ郡 LZWアルゴリズムによる
    int block_num;//イメージデータの個数
    unsigned char *block_sizes;//イメージデータのサイズリスト
    unsigned char *image_data;//イメージデータ郡。 LZWアルゴリズムによる
} image;

typedef struct {// 1対のgc_extension,imageで1つのframeとする
    gc_extension gcblock;
    image imageblock;
} frame;

typedef struct {//gif型を定義
    gif_header gifheader;
    int has_appext;
    app_extension appextension;
    int frame_count;
    frame *frames;
} gif;

char *getPath(char* file);//利用可能ファイルを一覧表示、ファイルを選択させ、ファイル名を返す

int isGif(char *file);//ファイルの拡張子がgifかどうか調べる

int analyzeGIF(gif* analyze, FILE *fp);//ポインタで指定された変数にgif型の変数を代入する、戻り値: 成功=0 失敗=1~

void playGIF(gif* gifp);//ポインタが示す先のgifを再生する //未実装

void destroyGIF(gif* gifp);//ポインタが示す先のgifが確保しているメモリを開放する

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    int loopflag = 0;

    do{
        char file[256];
        strcpy(file, getPath(file));

        FILE *fp;
        fp = fopen(file, "rb");
        if (fp == NULL){
            printf(">>File(%s) open failed\n", file);
            return 1;
        }
        printf(">>File(%s) open successed\n", file);

        int result;
        gif analyzed;
        result = analyzeGIF(&analyzed, fp);

        fclose(fp);

        if(result != 0){
            printf(">>GIF Analyzing failed\n");
            return 1;
        }
        printf(">>GIF Analyzing successed\n");

        int select;
        int menu_flag = 1;
        printf("------Controll Menu------\n");
        printf("0: (Not implemented) Play GIF Animation\n");
        printf("1: Analyze other GIF\n");
        printf("2: Quit\n");
        printf("-------------------------\n");

        while(menu_flag){
            printf("Please enter menu number: ");
            scanf("%d", &select);
            switch(select){

                case 0:
                    playGIF(&analyzed);
                    break;

                case 1:
                    menu_flag = 0;
                    loopflag = 1;
                    break;

                case 2:
                    printf("Bye\n");
                    menu_flag = 0;
                    loopflag = 0;
                    break;

                default:
                    printf("It is invalid number\n");
                    break;
            }
        }

        //GIF破棄
        destroyGIF(&analyzed);
    }while(loopflag == 1);

    return 0;
}

char *getPath(char* file){
    printf("----Available files----\n");

    DIR *dir; //ディレクトリポインタ
    struct dirent *ds;//ディレクトリストリーム構造体
    char path[64] = ".\\"; //カレントディレクトリ

    char files[64][256];//ファイルリスト
    int i = 0;

    dir = opendir(path);//ディレクトリを開く //dirにはディレクトリポインタが代入される
    for(ds = readdir(dir); ds != NULL; ds = readdir(dir)){//ディレクトリを表すdirent構造体へのポインタを返し、末尾に達するorエラーを吐くとNULLになってループ終了
        if(strcmp(ds->d_name, ".") != 0 && strcmp(ds->d_name, "..") != 0 && isGif(ds->d_name)){
            printf("%d: %s\n", i, ds->d_name);//構造体のメンバ変数d_nameを表示
            strcpy(files[i], ds->d_name);
            i++;
        }
    }
    closedir(dir);//ディレクトリを閉じる //ディレクトリポインタを引数として渡す

    printf("-----------------------\n");

    if(i == 0){//利用可能ファイルがなかった場合、アプリケーションを終了
        printf("Available file was not found");
        exit(1);
    }

    int select;

    while(1){
        printf("Please enter file number: ");
        scanf("%d", &select);//ファイル番号の入力

        if(select < 0 || i <= select){
            printf("It is invalid Number\n");
            continue;
        }

        break;
    }

    strcpy(file, files[select]);

    return file;
}

int isGif(char *file){
    return strcmp(strrchr(file, '.'), ".gif") == 0 ? 1 : 0;
}

int analyzeGIF(gif* analyze, FILE *fp){
    printf(">> Start GIF Analyzing...\n");

    /*ヘッダの解析部分*/
    printf(">> Start Header Analyzing...\n");
    gif_header header;

    //Signature(3 Bytes) "GIF"で固定
    fread(header.signature, 1, 3, fp); header.signature[3] = '\0';
    printf("|Signature : %s\n", header.signature);
    if(strcmp(header.signature, "GIF") != 0) return 1;

    //Version(3 Bytes) "87a" or "89a" //(今回は89aのみ対応にした)->87aにも対応
    fread(header.version, 1, 3, fp); header.version[3] = '\0';
    printf("|Version : %s\n", header.version);
    /*if(strcmp(header.version, "89a") != 0){
        printf("|Version %s is not supported(89a is only supported)\n", header.version);
        return 2;
    }*/

    //Logical Screen Width(2 Bytes), Logical Screen Height(2 Bytes) 0x1234 の場合は 0x34 0x12 と格納される。
    header.width = fgetc(fp) + fgetc(fp)*0x1000;
    header.height = fgetc(fp) + fgetc(fp)*0x1000;
    printf("|Width : %d Height : %d\n", header.width, header.height);

    //GCTF(1b)	CR(3b)	SF(1b)	SGCT(3b)
    unsigned char bits = fgetc(fp);
    header.gctf = bits >> 7;//Global Color Tableが存在する場合は1、存在しない場合は0。
    header.cr = ((bits >> 4) & 7) + 1;//画像1ドットを表わすのに必要なビット数
    header.sf = (bits >> 3) & 1;//Global Color Tableがソートされている場合は1、ソートされていない場合は0。
    header.sgct = pow(2, (bits & 0x07) + 1);//Global Color Tableの個数
    printf("|Flags: %d (GCTF: %d CR: %d SF: %d SGCT: %d)\n", bits, header.gctf, header.cr, header.sf, header.sgct);

    //Background Color Index(1 Byte)
    header.bgcolor = fgetc(fp);
    printf("|Background Color Index : %d\n", header.bgcolor);

    //Pixel Aspect Ratio(1 Byte) この値(1～255)をnとし、(n+15)/64 が実際の比率となる。値0はこの比率情報が与えられていないことを意味する。//とりあえず0のみに対応
    header.paratio = fgetc(fp);
    printf("|Pixel Aspect Ratiop : %d\n", header.paratio);

    if(header.gctf == 1){//Global Color Table Flagが1の場合に存在する。1つの色情報につきRGBの3バイトずつ、2の(Size of Global Color Table)乗個並ぶ。
        printf(">>Global Color Table is exist, Start indexing...\n");
        header.gctable = (rgb*)malloc(sizeof(rgb) * header.sgct);
        for(int i = 0; i < header.sgct; i++){
            rgb rgbtemp;
            rgbtemp.r = fgetc(fp);
            rgbtemp.g = fgetc(fp);
            rgbtemp.b = fgetc(fp);
            header.gctable[i] = rgbtemp;
            //printf(">> %d: r:%d g:%d b:%d\n", i, rgbtemp.r, rgbtemp.g, rgbtemp.b);
        }
        printf(">>Blobal Color Table indexing is finished\n");
        printf("|Global Color Table: %d\n", header.sgct);

    }else{
        printf(">> Color Table is NOT exist\n");
    }
    analyze->gifheader = header;
    printf(">> Header Analyzing successed\n");


    /*Block 解析部分*/
    printf(">> Blocks Analyzing...\n");

    int frame_count = 0;//フレーム数
    float duration = 0;//アニメーションの長さ
    unsigned char loopflag = 1;
    unsigned char type, label, block_size;
    analyze->has_appext = 0;
    frame *framestore = (frame*)malloc(sizeof(frame) * MAX_FRAME);//一旦大容量(MAX_FRAME)を確保
    while(loopflag){
        type = fgetc(fp);
        //printf(">>Type: 0x%x\n", type);
        switch(type){

            case 0x3b://Trailer
                //printf(">>Trailer found\n");
                loopflag = 0;
                break;

            case 0x2c://Image Block
                {
                    //printf(">>Image Block found\n");
                    image imageblock;
                    imageblock.pos_left = fgetc(fp) + fgetc(fp)*0x1000;//Image Left Position(2B)
                    imageblock.pos_top = fgetc(fp) + fgetc(fp)*0x1000;//Image Top Position(2B)
                    imageblock.width = fgetc(fp) + fgetc(fp)*0x1000;//Image Width(2B)
                    imageblock.height = fgetc(fp) + fgetc(fp)*0x1000;//Image Height(2B)
                    unsigned char flags = fgetc(fp);//LCTF(1b) IF(1b) SF(1b) R(2b) SLCT(3b) 計1Byte
                    imageblock.lct_flag = flags >> 7;
                    imageblock.i_flag = (flags >> 6) & 1;
                    imageblock.sort_flag = (flags >> 5) & 1;
                    imageblock.slct = pow(2, (flags & 0x07) + 1);
                    if(imageblock.lct_flag == 1){
                        imageblock.lctable = (rgb*)malloc(sizeof(rgb) * imageblock.slct);
                        for(int i = 0; i < imageblock.slct; i++){//Local Color Table(0～255×3 Bytes)
                            rgb rgbtemp;
                            rgbtemp.r = fgetc(fp);
                            rgbtemp.g = fgetc(fp);
                            rgbtemp.b = fgetc(fp);
                            imageblock.lctable[i] = rgbtemp;
                        }
                    }
                    imageblock.lzw_min_bit = fgetc(fp);//LZW Minimum Code Side(1 Byte) LZWコードの最小ビット数。
                    
                    char *block_data = (char*)malloc(sizeof(char) * 256 * 30);//とりあえず多くとっておく

                    unsigned char block_sizes[256] = {0};
                    int blocksize_sum = 0;
                    int block_count = 0;
                    int block_data_count = 0;
                    for(int i = 0; 1; i++){
                        block_sizes[i] = fgetc(fp);//1byte(1~255)
                        blocksize_sum += block_sizes[i];
                        if(block_sizes[i] == 0x00) break;//0はBlock Terminatorを表わす
                        block_count++;
                        for(int j = 0; j < block_sizes[i]; j++){
                            block_data[block_data_count] = fgetc(fp);//イメージデータ部へ格納
                            block_data_count++;
                        }
                    }
                    
                    imageblock.block_num = block_count;
                    imageblock.block_sizes = (char*)malloc(sizeof(char) * block_count);
                    imageblock.image_data = (char*)malloc(blocksize_sum);

                    int counter = 0;
                    for(int i = 0; block_sizes[i] != 0x00; i++){ 
                        imageblock.block_sizes[i] = block_sizes[i];
                        for(int j = 0; j < block_sizes[i]; j++){
                            imageblock.image_data[counter] = block_data[counter];
                            counter++;
                        }
                    }
                    free(block_data);//多くとった分を破棄
                    
                    framestore[frame_count].imageblock = imageblock;//フレームへ代入
                    frame_count++;
                    if(frame_count == MAX_FRAME){
                        loopflag = 0;
                    }
                }
                break;

            case 0x21://>Extension Block
                label = fgetc(fp);
                switch(label){

                    case 0xf9://Graphic Control Extension
                        {
                            //printf(">>Graphic Control Extension Block found\n");
                            gc_extension gcext;
                            fgetc(fp);//Block Size(1 Byte) 0x04 の固定値
                            fgetc(fp);//R(3b) DM(3b) UIF(1b) TCF(1b) 計1Byte
                            fread(&gcext.delay_time, 2, 1, fp);//for(int j = 0; j < 2; j++) fgetc(fp);//Delay Time(2 Bytes) 表示する際の遅延時間(100分の1秒単位)。
                            fgetc(fp);//Transparent Color Index(1 Byte)
                            fgetc(fp);//Block Terminator(1 Byte) ブロック並びの終わりを表わす。0x00 固定値。
                            duration += gcext.delay_time;

                            framestore[frame_count].gcblock = gcext;//フレームへ代入
                        }
                        break;
                    
                    case 0xfe://Comment Extension //とりあえず無視
                        //printf(">>Comment Extension Block found\n");
                        while(1){
                            block_size = fgetc(fp);//1byte(1~255)
                            if(block_size == 0x00) break;//0はBlock Terminatorを表わす
                            for(int j = 0; j < block_size; j++) fgetc(fp);//Comment Data(nB)
                        }
                        break;

                    case 0x01://Plain Text Extension //とりあえず無視
                        {
                            //printf(">>Plain Text Extension Block found\n");
                            fgetc(fp);//Block Size #1(1 Byte) 0x0c の固定値
                            for(int j = 0; j < 2; j++) fgetc(fp);//Text Grid Left Position(2B)
                            for(int j = 0; j < 2; j++) fgetc(fp);//Text Grid Top Position(2B)
                            for(int j = 0; j < 2; j++) fgetc(fp);//Text Grid Width(2B)
                            for(int j = 0; j < 2; j++) fgetc(fp);//Text Grid Height(2B)
                            fgetc(fp);//Character Cell Width(1B)
                            fgetc(fp);//Character Cell Height(1B)
                            fgetc(fp);//Text Foreground Color Index(1B)
                            fgetc(fp);//Text Background Color Index(1B)
                            char block_size_sum;
                            while(1){
                                block_size = fgetc(fp);//1byte(1~255)
                                block_size_sum += block_size;
                                if(block_size == 0x00) break;//0はBlock Terminatorを表わす
                                for(int j = 0; j < block_size; j++) fgetc(fp);//Plain Text Data(n Byte)
                            }
                        }
                        break;

                    case 0xff://Application Extension
                        {
                            //printf(">>Application Extension Block found\n");
                            app_extension appext;
                            fgetc(fp);//Block Size #1(1 Byte) 0x0b の固定値
                            fread(appext.app_indentifier, 8, 1, fp); appext.app_indentifier[8] = '\0';//Application Identifier(8 Bytes)
                            fread(appext.app_authcode, 3, 1, fp); appext.app_authcode[3] = '\0';//Application Authentication Code(3 Bytes)

                            int data_size = 0;
                            int data_num = 0;
                            char data_sizes_temp[256];//各データのサイズ配列
                            char data_temp[256];//データを順に入れていく配列
                            int data_counter = 0;
                            for(int j = 0; 1; j++){
                                data_sizes_temp[j] = fgetc(fp);//1byte(1~255)
                                if(data_sizes_temp[j] == 0x00){//0はBlock Terminatorを表わす
                                    data_num = j;
                                    break;
                                }
                                data_size += data_sizes_temp[j];
                                for(int k = 0; k < data_sizes_temp[j]; k++){
                                    data_counter += k;
                                    data_temp[data_counter] = fgetc(fp);//Application Data(nB)
                                }
                            }
                            appext.app_data_sizes = (char*)malloc(sizeof(char) * data_num);
                            appext.app_data = (char*)malloc(sizeof(char) * data_counter);
                            for(int i = 0; i < data_num; i++) appext.app_data_sizes[i] = data_sizes_temp[i];
                            for(int i = 0; i < data_counter; i++) appext.app_data[i] = data_temp[i];

                            analyze->has_appext = 1;
                            analyze->appextension = appext;
                        }
                        break;                   

                }
                break;
        }
    }

    analyze->frame_count = frame_count;
    analyze->frames = (frame*)malloc(sizeof(frame) * frame_count);
    for(int i = 0; i < frame_count; i++){
        analyze->frames[i] = framestore[i];
    }
    free(framestore);

    printf("|Animation Duration: %gs\n", duration / 100);
    printf("|FrameCount: %d\n", frame_count);
    printf(">> Blocks Analyzing Successed\n");
    return 0;
}

void playGIF(gif* gifp){
    /*
    LZW圧縮形式でイメージデータが保存されており、解析が難しく、時間的、技術的に厳しかった為断念
    夏休みに知識をつけて完成させたい。
    https://qiita.com/7shi/items/33117c6c369d37dc6cdd これを参考にすればできそう
    */
    /*frame testframe = gifp->frames[0];
    for(int i = 0; i < testframe.imageblock.block_sizes[0]; i++){
        printf("0x%x ", testframe.imageblock.image_data[i]);
        if((i + 1) % 8 == 0){
            printf("\n");
        }
    }
    printf("\n");
    printf("%d\n", testframe.imageblock.image_data[testframe.imageblock.block_sizes[0]]);*/
    printf("This function is not yet implemented\n");
    return;
}

void destroyGIF(gif* gifp){
    //gif_headerの開放
    if(gifp->gifheader.gctf == 1){
        free(gifp->gifheader.gctable);
    }
    //frameの開放
    for(int i = 0; i < gifp->frame_count; i++){
        //各frame内の開放
        if(gifp->frames[i].imageblock.lct_flag == 1) free(gifp->frames[i].imageblock.lctable);
        free(gifp->frames[i].imageblock.block_sizes);
        free(gifp->frames[i].imageblock.image_data);
    }
    free(gifp->frames);
    //app_extensionの開放
    if(gifp->has_appext == 1){
        //サイズリスト、データ群を開放
        free(gifp->appextension.app_data_sizes);
        free(gifp->appextension.app_data);
    }
    //gif自体を開放
    free(gifp);
    return;
}