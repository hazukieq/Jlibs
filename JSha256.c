/**
 * 代码参考自<CSDN>
 * 原作者：qq_43176116
 * 原地址：https://blog.csdn.net/qq_43176116/article/details/110388321
 * 修改: 叶月绘梨依
 * 日期：2023年6月24日夜
 */
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "JSha256.h"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define copy_uint32(p, val) *((uint32_t *)p) = __builtin_bswap32((val))//gcc 内建函数__builtin_bswap32，
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define copy_uint32(p, val) *((uint32_t *)p) = (val)
#else
#error "Unsupported target architecture endianess!"
#endif

//将任意大小的字符分割成32-bit的字
#define SPLIT_32H(block) ((uint32_t)((block)[0]&255)<<24)|((uint32_t)((block)[1]&255)<<16)|\
			    ((uint32_t)((block)[2]&255)<<8)|((uint32_t)((block)[3]&255));

#define Ch(x,y,z) (x&y)^(~x&z)
#define Ma(x,y,z) (x&y)^(x&z)^(y&z)
//循环右移
#define Cyshiftr(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define Sigma0(x) (Cyshiftr(x,2)^Cyshiftr(x,13)^Cyshiftr(x,22))
#define Sigma1(x) (Cyshiftr(x,6)^Cyshiftr(x,11)^Cyshiftr(x,25))
#define Gamma0(x) (Cyshiftr(x,7)^Cyshiftr(x,18)^((x)>>3))
#define Gamma1(x) (Cyshiftr(x,17)^Cyshiftr(x,19)^((x)>>10))


static const uint32_t k[64] = {
     0x428a2f98,  0x71374491,  0xb5c0fbcf,  0xe9b5dba5,  0x3956c25b, 
     0x59f111f1,  0x923f82a4,  0xab1c5ed5,  0xd807aa98,  0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
    0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 
    0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 
    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 
    0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 
    0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 
    0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/**
 * 加密流程:
 * 1. 消息预处理
 * - 将原始信息转化为二进制编码
 * - 在其后面补个'1'(0b1000 0000)
 * - 补充 N 个0，使得其总长度取模得448,
 *   (l+1+k) mod 512 = 448，l:原始消息长度,k:填充0长度
 * 2. 附加原始消息的长度值
 * 3. 消息拆分为 N 个 512-bit的区(block)
 * - 对于每个区，构造64个字(根据其消息构造出16个字,剩下48个根据前16个字和迭代公式得到)
 * - 完成64次加密迭代
 * 4.得到256比特的加密数据 
 */
static void sha256(const unsigned char *data, size_t len, unsigned char *out) {
	uint32_t h0 = 0x6a09e667;
        uint32_t h1 = 0xbb67ae85;
        uint32_t h2 = 0x3c6ef372;
        uint32_t h3 = 0xa54ff53a;
        uint32_t h4 = 0x510e527f;
        uint32_t h5 = 0x9b05688c;
        uint32_t h6 = 0x1f83d9ab;
        uint32_t h7 = 0x5be0cd19;
    
	//公式 y mod 512=x...r
        //需要补齐余数为448
	//即使长度已经满足对512取模后余数是448，补位也必须要进行，这时要填充512个比特。因此，填充是至少补一位，最多补512位
	//故 r>448 => y=448+512-r or y=448-r
	int r=(int)((len*8)%512);
        int append_bits=(r < 448) ? 448 - r : 448 + 512 - r;
        int append = append_bits / 8;
        size_t new_len = len + append + 8;// 原始数据+填充+64bit位数(64/8=>8)
        
        unsigned char buf[new_len];
        //将内存（字符串）前n个字节清零
	//按照SHA256算法流程,需要在添加以1开头的比特后面补齐足够的0000 0000以被512取模得余数448
	//由于之前已经计算出新长度new_len (原字符长度+补齐0的长度+长度信息8)
	//故可以直接将扩容后的内存以0x0进行格式化
	bzero(buf + len, append); 
	if (len > 0) {
    	    memcpy(buf, data, len);
        }
        
	//按照惯例,原字符内容后需要添加以1开头的比特
        buf[len] = (unsigned char)0x80;//0x80->0b1000 0000
        
	//len是字节数,故需乘以8=>实际总比特数
	//添加64-bit的8字节字符串,其是原字符的长度信息
        uint64_t bits_len = len * 8;
	//通过循环将64-bit(8字节)的长度信息按每8-bit添加到buf中
        for (int i = 0; i < 8; i++) {
    	    //0xff(255)是mask，任何位与其组合都得本身,且最终值会被格式化为1个8位字节
    	    //例如：0b0000 10010&0xff(ob1111 1111)->0b0000 10010
    	    buf[len + append + i] = (bits_len >> ((7 - i) * 8)) & 0xff;
        }
	
        //存储区块加密用到的64个字,即64个32bit(4字节)大小
	uint32_t w[64];
	bzero(w, 64);//格式化

        //按512bit(512/8=>64 字节)分割区块
        //注意new_len是字节数,不是bits总长度！！
        size_t chunk_len = new_len / 64; 
        
	//对每个区块进行以下加密迭代
	for (int idx = 0; idx < chunk_len; idx++) {
	    uint32_t val = 0;
	    //构造64个32-bit字
	    //前16个字由消息的第i个区块得到
	    //剩下48个字由迭代公式得到
	    
	    //将一个区块分解为16个32-bitのbig-endianの字,记为w[0],...,w[15]
	    for(int i=0;i<16;i++){
		    //idex*64 表明当前是哪个区块(每64字节分成一个区块)
		    //i*4 表明一个字需要根据区块32 bits 来构造
		    unsigned char* block=buf+(4*i+idx*64);
		    val=SPLIT_32H(block);
		    w[i]=val;
		    val=0;
	    }
	    
	    //其余的字由如下迭代公式得到
	    for (int i = 16; i < 64; i++) {            
		    w[i]=Gamma1(w[i-2])+w[i-7]+Gamma0(w[i-15])+w[i-16];
	    }

	    //进行64次加密循环
            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;
            uint32_t f = h5;
            uint32_t g = h6;
            uint32_t h = h7;
            for (int i = 0; i < 64; i++) {
		    //表示e经过Sigma1函数输出结果s_1
		    uint32_t s_1=Sigma1(e);
		    
		    //表示e,f,g经过Ch函数输出结果ch
		    uint32_t ch = Ch(e,f,g);
                    
		    //表示ch+h+w_i+k_i 相加输出tmp1
		    uint32_t temp1 = h+s_1+ch+k[i]+w[i];
                    
		    //表示a经过函数Sigma0函数输出s_0
                    uint32_t s_0=Sigma0(a);
		    
		    //表示a,b,c经过Ma函数输出maj
                    uint32_t maj=Ma(a,b,c);
		    
		    //表示maj+s_0 相加输出tmp2
		    uint32_t temp2 = s_0 + maj;
                    
		    h = g;
                    g = f;
                    f = e;
                    e = d + temp1;
                    d = c;
                    c = b;
                    b = a;
                    a = temp1 + temp2;
	    }
            
	    h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;
    }
	//得到8位32-bit加密信息
	//8*32=256 bits
	copy_uint32(out, h0);
    	copy_uint32(out + 1, h1);
    	copy_uint32(out + 2, h2);
    	copy_uint32(out + 3, h3);
    	copy_uint32(out + 4, h4);
    	copy_uint32(out + 5, h5);
    	copy_uint32(out + 6, h6);
    	copy_uint32(out + 7, h7);
}

//需要手动释放内存！！！
void jsha_hash(const char* data,size_t len, unsigned char** outptr_ptr){
	if(data==NULL) {
		printf("data cannot be NULL\n");
		return;
	}

	*outptr_ptr=(unsigned char*)malloc(32);
	if(*outptr_ptr==NULL){
		printf("you must pass a non-NULL pointer address to jsha_hash\n");	
		return;
	}

	sha256((const unsigned char*)data,len,*outptr_ptr);
	if(!*outptr_ptr) {
		printf("cannot copy hash value to outptr_ptr");
		free(outptr_ptr);
		return;
	}
}

void jsha_getobj(const char *data, size_t len, shacontext **contextptr_ptr){
	if(data==NULL) {
		printf("data cannot be NULL\n");
		return;
	}
	
	*contextptr_ptr=(shacontext*)malloc(sizeof(shacontext)+sizeof(char)*(len+1));
	if(*contextptr_ptr==NULL){
		printf("you must pass a non-NULL pointer address to jsha_hash\n");	
		return;
	}
	
	unsigned char* out_data;
	jsha_hash(data,len,&out_data);
	memcpy((*contextptr_ptr)->hash,out_data,32);

	char* bindata=jsha_getbin(out_data,0x00);
	memcpy((*contextptr_ptr)->bin,bindata,257);

	char* hexdata=jsha_gethex(out_data,0x00);
	memcpy((*contextptr_ptr)->hex,hexdata,65);

	memcpy((*contextptr_ptr)->raw,data,len);
	(*contextptr_ptr)->raw[len]='\0';
	
	if(out_data) free(out_data);
	if(bindata) free(bindata);
	if(hexdata) free(hexdata);
}

void jsha_printobj(shacontext *context){
	if(context==NULL) return;
	printf("\033[0;32m\n{\n\"bin\": \"%s\",\n\"hex\": \"%s\",\n\"raw\": \"%s\"\n}\n\n\033[0m",context->bin,context->hex,context->raw);
}

char* jsha_getjson(shacontext *context){
	if(context==NULL) return NULL;
	char* json;
	sprintf(json,"{\n\t\"hex\":\"%s\",\n\t\"raw\":\"%s\"\n}",context->hex,context->raw);
	return json;
}

/*
 * 不想用分隔符的话,请将split_tag置为0x00
 * 需要手动释放内存！！！
 */
char* jsha_getbin(const unsigned char *hash,char split_tag){
	int istag_mode=0;
	if(split_tag>0x00) istag_mode=1;
	
	
	int size=istag_mode?32*8+32+1:32*8+1;
	char* binptr_ptr=(char*)malloc(size);
	if(binptr_ptr==NULL) return NULL;

	int cnt_j=istag_mode?9:8;
	for(int i=0;i<32;i++){
		for(int j=0;j<cnt_j;j++){
				binptr_ptr[i*cnt_j+j]=(hash[i]>>(7-j)&1)?'1':'0';
				//为什么要小于247?因为最后一轮的8位数并不需要添加split_tag了。故小于31*8=248即可
				if(j==8&&istag_mode&&j*i<248) binptr_ptr[i*cnt_j+j]=split_tag;
		}
	}
	binptr_ptr[size-1]='\0';
	return binptr_ptr;
}

/*
 * 不想用分隔符的话,请将split_tag置为0x00
 * 需要手动释放内存！
 */
char* jsha_gethex(const unsigned char* hash,char split_tag){
	int istag_mode=0;
	if(split_tag>0x00) istag_mode=1;

	//因为hashptr_ptr指向char*指针地址,所以*hashptr_ptr实际上是一个char数组指针
	//即 hashptr_ptr存储了一个主函数中char* hashes的指针地址,这样就可以对其进行操作了
	//故可以对一个char数组指针分配内存
	int size=istag_mode?65+31:65;
	char* hexptr_ptr=malloc(size);
	if(hexptr_ptr==NULL) return NULL;
	
	char s[4]={0};
	for(int i=0;i<32;i++){
		if(istag_mode){
			sprintf(s,"%02x%c", hash[i],split_tag);
			memcpy(hexptr_ptr+i*3,s,3);
		}
		else{ 
			sprintf(s,"%02x",hash[i]);
			memcpy(hexptr_ptr+i*2,s,2);
			memset(s,0,4);
		}
	}
	hexptr_ptr[size-1]='\0';
	return hexptr_ptr;
}

/**
 * fmt_mode: heximal mode 0,binary mode 1:w 
 * split_tag: any
 */
void jsha_print(const unsigned char* out,int fmt_mode,char* split_tag){
	if(out==NULL) return;
	if(fmt_mode>1) fmt_mode=1;
	char tag=' ';
	int istag_mode=0;
	if(split_tag!=NULL) {
		tag=split_tag[0];
		istag_mode=1;
	}

	switch(fmt_mode){
		case 0:
			printf("\033[0;32m\t");
			for(int i=0;i<32;i++){
				for(int j=0;j<8;j++)
					printf("%c",(out[i]>>(7-j)&1)==1?'1':'0');
				if(i<31&&istag_mode) printf("%c",tag);
			}
			printf("\n\033[0m");
			break;
		case 1:
			printf("\033[0;32m\t");
			for(int i=0;i<32;i++){
				printf("%02x",out[i]);
				if(i<31&&istag_mode) printf("%c",tag);
			}
			printf("\n\033[0m");
			break;

		default:
			break;
	}
}

void jsha_test(){
	//--PASS jsha_hash--/
	const char h[]="hello";
	unsigned char* out=NULL;
	jsha_hash(h,strlen(h), &out);
	unsigned int index = *((unsigned int*)out);
	printf("hash_index:%d\n",index%100);	
	if(out) free(out);
	//--jsha_hash--//
	
	//--PASS jsha_gethex--/
	const char lh[]="hello";
	unsigned char* lout=NULL;
	jsha_hash(lh,strlen(lh), &lout);
	char* hashes=jsha_gethex(lout,0);
	
	printf("hash_hexfmt: %s\n",hashes);	
	if(lout) free(lout);
	if(hashes) free(hashes);
	//--jsha_gethex--//
	
	//--PASS jsha_print--/
	unsigned char* pout=NULL;
	jsha_hash("hello",strlen("hello"),&pout);
	jsha_print(pout,1,".");
	if(pout) free(pout);
	//--jsha_print--//
	
	//--PASS Sha256Context---//
	shacontext* context=NULL;
	jsha_getobj("hello",strlen("hello"),&context);
	jsha_printobj(context);
	printf("\033[0;32m%s\n\033[0m",jsha_getjson(context));
	if(context) free(context);
	//--PASS Sha256Context---//
	
	//--PASS jsha_getbin--/
	unsigned char* hout=NULL;
	jsha_hash("hello",strlen("hello"),&hout);
	char* bins=jsha_getbin(hout,0x00);
	printf("hash_binfmt: %s\n",bins);
	if(hout) free(hout);
	if(bins) free(bins);
	//--jsha_getbin--//
}


/*int main(void){
	jsha_test();
	return 0;
}*/

