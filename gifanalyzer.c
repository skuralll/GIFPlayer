#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>

/*
�Q�l����
�\���̂ɂ���: https://9cguide.appspot.com/16-02.html
�J�����g�f�B���N�g�����̃t�@�C�����ꗗ�\��: https://torisky.com/
GIF�t�H�[�}�b�g�̏ڍ�: http://www.tohoho-web.com/wwwgif.htm
GIF�t�H�[�}�b�g�̎擾�v���O������(�摜��Width,Height�擾���������݂̂��Q�l�ɂ���): https://reerishun.com/makerblog/?p=693
*/

/*
Block List
0x2c        Image Block
0x21 0xf9   Graphic Control Extension
0x21 0xfe   Comment Extension
0x21 0x01   Plain Text Extension
0x21 0xff   Application Extension

0x21 �͊g���u���b�N�ł��邱�Ƃ�����

0x3b Trailer �I�� (1Byte) Block�ł͂Ȃ�
*/

const int MAX_FRAME = 512;//�ő�t���[����

typedef struct{//rgb�^���`
    unsigned char r, g, b;
} rgb;

typedef struct {//gif_header�^���`
    char signature[4];//GIF
    char version[4];//�o�[�W����
    unsigned int width;//�摜��
    unsigned int height;//�摜����
    int gctf, cr, sf, sgct;//�e��t���O�Ȃ�
    int bgcolor;//�w�i�F�̃C���f�b�N�X
    int paratio;//�s�N�Z���̏c����
    rgb *gctable;//rgb gctable[256]; //�O���[�o���J���[�e�[�u�� (gctf��1�̏ꍇ�ɑ���)
} gif_header;

typedef struct{//app_extension�^���` Application Extension
    unsigned char app_indentifier[9];//Application Identifier(8 Bytes) + null = 9Byte "NETSCAPE"
    unsigned char app_authcode[4];//Application Authentication Code(3 Bytes) + null = 4Byte "2.0"
    unsigned char *app_data_sizes;//�A�v���P�[�V�����f�[�^�̃T�C�Y���X�g
    unsigned char *app_data;//�A�v���P�[�V�����f�[�^�S�B 1�o�C�g�� 0x01 �����2�o�C�g�̃��[�v��(0�`65535) 
} app_extension;

typedef struct{//gc_extension�^���`  Graphic Control Extension
    unsigned short delay_time;//Delay Time(2 Bytes)
} gc_extension;

typedef struct{//image�^���`  Image Block
    unsigned int pos_left; //GIF�摜�S�̂ɑ΂��邱�̃C���[�W�u���b�N�̍��[���Έʒu
    unsigned int pos_top; //GIF�摜�S�̂ɑ΂��邱�̃C���[�W�u���b�N�̏�[���Έʒu
    unsigned int width;//�C���[�W�u���b�N��
    unsigned int height;//�C���[�W�u���b�N����
    int lct_flag, i_flag, sort_flag, slct;//�e��t���O 
    rgb *lctable;//rgb lctable[256];//���[�J���J���[�e�[�u�� (lct_flag��1�̏ꍇ�ɑ���)
    unsigned char lzw_min_bit;//LZW�R�[�h�̍ŏ��r�b�g���B
    //unsigned char image_data[256][256];//�C���[�W�f�[�^�S LZW�A���S���Y���ɂ��
    int block_num;//�C���[�W�f�[�^�̌�
    unsigned char *block_sizes;//�C���[�W�f�[�^�̃T�C�Y���X�g
    unsigned char *image_data;//�C���[�W�f�[�^�S�B LZW�A���S���Y���ɂ��
} image;

typedef struct {// 1�΂�gc_extension,image��1��frame�Ƃ���
    gc_extension gcblock;
    image imageblock;
} frame;

typedef struct {//gif�^���`
    gif_header gifheader;
    int has_appext;
    app_extension appextension;
    int frame_count;
    frame *frames;
} gif;

char *getPath(char* file);//���p�\�t�@�C�����ꗗ�\���A�t�@�C����I�������A�t�@�C������Ԃ�

int isGif(char *file);//�t�@�C���̊g���q��gif���ǂ������ׂ�

int analyzeGIF(gif* analyze, FILE *fp);//�|�C���^�Ŏw�肳�ꂽ�ϐ���gif�^�̕ϐ���������A�߂�l: ����=0 ���s=1~

void playGIF(gif* gifp);//�|�C���^���������gif���Đ����� //������

void destroyGIF(gif* gifp);//�|�C���^���������gif���m�ۂ��Ă��郁�������J������

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

        //GIF�j��
        destroyGIF(&analyzed);
    }while(loopflag == 1);

    return 0;
}

char *getPath(char* file){
    printf("----Available files----\n");

    DIR *dir; //�f�B���N�g���|�C���^
    struct dirent *ds;//�f�B���N�g���X�g���[���\����
    char path[64] = ".\\"; //�J�����g�f�B���N�g��

    char files[64][256];//�t�@�C�����X�g
    int i = 0;

    dir = opendir(path);//�f�B���N�g�����J�� //dir�ɂ̓f�B���N�g���|�C���^����������
    for(ds = readdir(dir); ds != NULL; ds = readdir(dir)){//�f�B���N�g����\��dirent�\���̂ւ̃|�C���^��Ԃ��A�����ɒB����or�G���[��f����NULL�ɂȂ��ă��[�v�I��
        if(strcmp(ds->d_name, ".") != 0 && strcmp(ds->d_name, "..") != 0 && isGif(ds->d_name)){
            printf("%d: %s\n", i, ds->d_name);//�\���̂̃����o�ϐ�d_name��\��
            strcpy(files[i], ds->d_name);
            i++;
        }
    }
    closedir(dir);//�f�B���N�g������� //�f�B���N�g���|�C���^�������Ƃ��ēn��

    printf("-----------------------\n");

    if(i == 0){//���p�\�t�@�C�����Ȃ������ꍇ�A�A�v���P�[�V�������I��
        printf("Available file was not found");
        exit(1);
    }

    int select;

    while(1){
        printf("Please enter file number: ");
        scanf("%d", &select);//�t�@�C���ԍ��̓���

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

    /*�w�b�_�̉�͕���*/
    printf(">> Start Header Analyzing...\n");
    gif_header header;

    //Signature(3 Bytes) "GIF"�ŌŒ�
    fread(header.signature, 1, 3, fp); header.signature[3] = '\0';
    printf("|Signature : %s\n", header.signature);
    if(strcmp(header.signature, "GIF") != 0) return 1;

    //Version(3 Bytes) "87a" or "89a" //(�����89a�̂ݑΉ��ɂ���)->87a�ɂ��Ή�
    fread(header.version, 1, 3, fp); header.version[3] = '\0';
    printf("|Version : %s\n", header.version);
    /*if(strcmp(header.version, "89a") != 0){
        printf("|Version %s is not supported(89a is only supported)\n", header.version);
        return 2;
    }*/

    //Logical Screen Width(2 Bytes), Logical Screen Height(2 Bytes) 0x1234 �̏ꍇ�� 0x34 0x12 �Ɗi�[�����B
    header.width = fgetc(fp) + fgetc(fp)*0x1000;
    header.height = fgetc(fp) + fgetc(fp)*0x1000;
    printf("|Width : %d Height : %d\n", header.width, header.height);

    //GCTF(1b)	CR(3b)	SF(1b)	SGCT(3b)
    unsigned char bits = fgetc(fp);
    header.gctf = bits >> 7;//Global Color Table�����݂���ꍇ��1�A���݂��Ȃ��ꍇ��0�B
    header.cr = ((bits >> 4) & 7) + 1;//�摜1�h�b�g��\�킷�̂ɕK�v�ȃr�b�g��
    header.sf = (bits >> 3) & 1;//Global Color Table���\�[�g����Ă���ꍇ��1�A�\�[�g����Ă��Ȃ��ꍇ��0�B
    header.sgct = pow(2, (bits & 0x07) + 1);//Global Color Table�̌�
    printf("|Flags: %d (GCTF: %d CR: %d SF: %d SGCT: %d)\n", bits, header.gctf, header.cr, header.sf, header.sgct);

    //Background Color Index(1 Byte)
    header.bgcolor = fgetc(fp);
    printf("|Background Color Index : %d\n", header.bgcolor);

    //Pixel Aspect Ratio(1 Byte) ���̒l(1�`255)��n�Ƃ��A(n+15)/64 �����ۂ̔䗦�ƂȂ�B�l0�͂��̔䗦��񂪗^�����Ă��Ȃ����Ƃ��Ӗ�����B//�Ƃ肠����0�݂̂ɑΉ�
    header.paratio = fgetc(fp);
    printf("|Pixel Aspect Ratiop : %d\n", header.paratio);

    if(header.gctf == 1){//Global Color Table Flag��1�̏ꍇ�ɑ��݂���B1�̐F���ɂ�RGB��3�o�C�g���A2��(Size of Global Color Table)����ԁB
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


    /*Block ��͕���*/
    printf(">> Blocks Analyzing...\n");

    int frame_count = 0;//�t���[����
    float duration = 0;//�A�j���[�V�����̒���
    unsigned char loopflag = 1;
    unsigned char type, label, block_size;
    analyze->has_appext = 0;
    frame *framestore = (frame*)malloc(sizeof(frame) * MAX_FRAME);//��U��e��(MAX_FRAME)���m��
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
                    unsigned char flags = fgetc(fp);//LCTF(1b) IF(1b) SF(1b) R(2b) SLCT(3b) �v1Byte
                    imageblock.lct_flag = flags >> 7;
                    imageblock.i_flag = (flags >> 6) & 1;
                    imageblock.sort_flag = (flags >> 5) & 1;
                    imageblock.slct = pow(2, (flags & 0x07) + 1);
                    if(imageblock.lct_flag == 1){
                        imageblock.lctable = (rgb*)malloc(sizeof(rgb) * imageblock.slct);
                        for(int i = 0; i < imageblock.slct; i++){//Local Color Table(0�`255�~3 Bytes)
                            rgb rgbtemp;
                            rgbtemp.r = fgetc(fp);
                            rgbtemp.g = fgetc(fp);
                            rgbtemp.b = fgetc(fp);
                            imageblock.lctable[i] = rgbtemp;
                        }
                    }
                    imageblock.lzw_min_bit = fgetc(fp);//LZW Minimum Code Side(1 Byte) LZW�R�[�h�̍ŏ��r�b�g���B
                    
                    char *block_data = (char*)malloc(sizeof(char) * 256 * 30);//�Ƃ肠���������Ƃ��Ă���

                    unsigned char block_sizes[256] = {0};
                    int blocksize_sum = 0;
                    int block_count = 0;
                    int block_data_count = 0;
                    for(int i = 0; 1; i++){
                        block_sizes[i] = fgetc(fp);//1byte(1~255)
                        blocksize_sum += block_sizes[i];
                        if(block_sizes[i] == 0x00) break;//0��Block Terminator��\�킷
                        block_count++;
                        for(int j = 0; j < block_sizes[i]; j++){
                            block_data[block_data_count] = fgetc(fp);//�C���[�W�f�[�^���֊i�[
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
                    free(block_data);//�����Ƃ�������j��
                    
                    framestore[frame_count].imageblock = imageblock;//�t���[���֑��
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
                            fgetc(fp);//Block Size(1 Byte) 0x04 �̌Œ�l
                            fgetc(fp);//R(3b) DM(3b) UIF(1b) TCF(1b) �v1Byte
                            fread(&gcext.delay_time, 2, 1, fp);//for(int j = 0; j < 2; j++) fgetc(fp);//Delay Time(2 Bytes) �\������ۂ̒x������(100����1�b�P��)�B
                            fgetc(fp);//Transparent Color Index(1 Byte)
                            fgetc(fp);//Block Terminator(1 Byte) �u���b�N���т̏I����\�킷�B0x00 �Œ�l�B
                            duration += gcext.delay_time;

                            framestore[frame_count].gcblock = gcext;//�t���[���֑��
                        }
                        break;
                    
                    case 0xfe://Comment Extension //�Ƃ肠��������
                        //printf(">>Comment Extension Block found\n");
                        while(1){
                            block_size = fgetc(fp);//1byte(1~255)
                            if(block_size == 0x00) break;//0��Block Terminator��\�킷
                            for(int j = 0; j < block_size; j++) fgetc(fp);//Comment Data(nB)
                        }
                        break;

                    case 0x01://Plain Text Extension //�Ƃ肠��������
                        {
                            //printf(">>Plain Text Extension Block found\n");
                            fgetc(fp);//Block Size #1(1 Byte) 0x0c �̌Œ�l
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
                                if(block_size == 0x00) break;//0��Block Terminator��\�킷
                                for(int j = 0; j < block_size; j++) fgetc(fp);//Plain Text Data(n Byte)
                            }
                        }
                        break;

                    case 0xff://Application Extension
                        {
                            //printf(">>Application Extension Block found\n");
                            app_extension appext;
                            fgetc(fp);//Block Size #1(1 Byte) 0x0b �̌Œ�l
                            fread(appext.app_indentifier, 8, 1, fp); appext.app_indentifier[8] = '\0';//Application Identifier(8 Bytes)
                            fread(appext.app_authcode, 3, 1, fp); appext.app_authcode[3] = '\0';//Application Authentication Code(3 Bytes)

                            int data_size = 0;
                            int data_num = 0;
                            char data_sizes_temp[256];//�e�f�[�^�̃T�C�Y�z��
                            char data_temp[256];//�f�[�^�����ɓ���Ă����z��
                            int data_counter = 0;
                            for(int j = 0; 1; j++){
                                data_sizes_temp[j] = fgetc(fp);//1byte(1~255)
                                if(data_sizes_temp[j] == 0x00){//0��Block Terminator��\�킷
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
    LZW���k�`���ŃC���[�W�f�[�^���ۑ�����Ă���A��͂�����A���ԓI�A�Z�p�I�Ɍ����������גf�O
    �ċx�݂ɒm�������Ċ������������B
    https://qiita.com/7shi/items/33117c6c369d37dc6cdd ������Q�l�ɂ���΂ł�����
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
    //gif_header�̊J��
    if(gifp->gifheader.gctf == 1){
        free(gifp->gifheader.gctable);
    }
    //frame�̊J��
    for(int i = 0; i < gifp->frame_count; i++){
        //�eframe���̊J��
        if(gifp->frames[i].imageblock.lct_flag == 1) free(gifp->frames[i].imageblock.lctable);
        free(gifp->frames[i].imageblock.block_sizes);
        free(gifp->frames[i].imageblock.image_data);
    }
    free(gifp->frames);
    //app_extension�̊J��
    if(gifp->has_appext == 1){
        //�T�C�Y���X�g�A�f�[�^�Q���J��
        free(gifp->appextension.app_data_sizes);
        free(gifp->appextension.app_data);
    }
    //gif���̂��J��
    free(gifp);
    return;
}