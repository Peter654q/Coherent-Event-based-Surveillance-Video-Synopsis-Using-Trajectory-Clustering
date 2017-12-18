#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

int init(int fn, int c_o);

int main(){
	double array1[20][8];
	double array2[20][8];
	double out[20][8];
	int frame = 9335;
	int counts_obj=0;
    
    counts_obj = init(25, counts_obj);

	for(int frameNumber=26; frameNumber<frame; frameNumber++){
			bool file_init = false;
			fstream fp1;
			stringstream ss1;
			ss1 << "out/F_out" << frameNumber-1 << ".txt";
			//ss1 << "F55.txt";
			string str1 = ss1.str();
			fp1.open(str1.c_str(), ios::in);

			fstream fp2;
			stringstream ss2;
			ss2 << "../txt/F" << frameNumber << ".txt";
			//ss2 << "F56.txt";
			string str2 = ss2.str();
			fp2.open(str2.c_str(), ios::in);
			if(!fp2){
				cout << frameNumber << " " << counts_obj << " ";
				while(true){
					frameNumber++;
					fstream fp2;
					stringstream ss2;
					ss2 << "../txt/F" << frameNumber << ".txt";
					//ss2 << "F56.txt";
					string str2 = ss2.str();
					fp2.open(str2.c_str(), ios::in);
					if(fp2){
						break;
					}
				}
				counts_obj = init(frameNumber, counts_obj);
				cout << frameNumber << " " << counts_obj << endl;
				file_init = true;
			}
			if(!file_init){
				// init the arrays
				for(int i=0;i<20;i++){
					for(int j=0;j<8;j++){
						array1[i][j]=0;
						array2[i][j]=0;
						out[i][j]=-1;
					}
				}			
				
				//put data into arrays
				for(int i=0;i<20;i++){
					for(int j=0;j<8;j++){
						fp1 >> array1[i][j];
						fp2 >> array2[i][j];
						//cout << array1[i][j] << " ";
						//cout << array2[i][j] << endl;
					}
				}

				//compare two arrays

				double nearest[2];
				for(int j=0;j<20;j++){
					nearest[0] = 10000;
					nearest[1] = 10000;
					for(int i=0;i<20;i++){
						if( array1[i][5]!=0 && array2[j][5]!=0 ){
							double dist = sqrt(pow((array1[i][7] - array2[j][7]),2) + pow((array1[i][6] - array2[j][6]),2));
							if (dist < nearest[1]){
								nearest[1]=dist;
								nearest[0]=array1[i][0];
							}
						}					
					}
					if (nearest[0] == 10000)
						break;
					//cout << nearest[0] << " ";	
					if(nearest[1]>150){
						out[j][0] = counts_obj;
						for(int i=1;i<8;i++)
							out[j][i] = array2[j][i];
						counts_obj++;
						cout << frameNumber << " " << counts_obj << endl;
					}else{
						out[j][0] = nearest[0];
						for(int i=1;i<8;i++)
							out[j][i] = array2[j][i];
					}
					
					//cout << out[j][0] << " ";
				}
				/*to compare each obj in same txt*/
				for(int j=0;j<20;j++){
					double now_onum = out[j][0];
					if(now_onum!=-1){
						for(int p=0;p<j;p++){
							double compare_onum = out[p][0];
							if(now_onum == compare_onum){
								double dist = sqrt(pow((out[j][7] - out[p][7]),2) + pow((out[j][6] - out[p][6]),2));
								if( dist>200){
									out[j][0] = counts_obj;
									counts_obj++;
									cout << frameNumber << " " << counts_obj << endl;
								}
							}
						}
					}
				}

				fstream fp_out;
				stringstream ss_out;
				ss_out << "out/F_out" << frameNumber << ".txt";
				string str_out = ss_out.str();
				fp_out.open(str_out.c_str(), ios::out|ios::app);
				for(int j=0;j<20;j++){
					if(out[j][0]==-1)
						break;
					fp_out << out[j][0] << endl;
					for(int i=1;i<8;i++)
						fp_out << out[j][i] << endl;
				}	
			}	
		
	}
				
	return 0;
}

int init(int fn, int c_o){
    int c_obj=c_o;
    fstream fp_in;
    stringstream ss1;
	ss1 << "../txt/F" << fn << ".txt";
	string str1 = ss1.str();
	fp_in.open(str1.c_str(), ios::in);

    fstream fp_out;
	stringstream ss_out;
	ss_out << "out/F_out" << fn << ".txt";
	string str_out = ss_out.str();
	fp_out.open(str_out.c_str(), ios::out|ios::app);

    double tmp;
    while(fp_in >> tmp){
        fp_out << c_obj << endl;
        c_obj++;
        for(int i=0;i<7;i++){
            fp_in >> tmp;
            fp_out << tmp << endl;
        }
    }
    return c_obj;
}
