#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

int main(){
	double array1[20][8];
	double array2[20][8];
	int frame = 200;
	int counts_obj=0;

	for(int frameNumber=25; frameNumber<frame; frameNumber++){
				fstream fp1;
			if(frameNumber==25){
				stringstream ss1;
				ss1 << "F" << frameNumber-1 << ".txt";
				//ss1 << "F55.txt";
				string str1 = ss1.str();
				fp1.open(str1.c_str(), ios::in);
			}else{
				stringstream ss1;
				ss1 << "F_out" << frameNumber-1 << ".txt";
				//ss1 << "F55.txt";
				string str1 = ss1.str();
				fp1.open(str1.c_str(), ios::in);
			}

			fstream fp2;
			stringstream ss2;
			ss2 << "F" << frameNumber << ".txt";
			//ss2 << "F56.txt";
			string str2 = ss2.str();
			fp2.open(str2.c_str(), ios::in);
			
			// init the arrays
			for(int i=0;i<20;i++){
				for(int j=0;j<8;j++){
					array1[i][j]=0;
					array2[i][j]=0;
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
			//init the number
			if(frameNumber==25){
				for(int i=0;i<20;i++){
					if(array1[i][0]!=0){
						array1[i][0]=i;
						counts_obj++;
					}else{
						break;					
					}
				}	
			}

			//compare two arrays

			double nearest[2];
			for(int j=0;j<20;j++){
				nearest[0] = 20;
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
				if (nearest[0] == 20)
					break;

				fstream fp_out;
				stringstream ss_out;
				ss_out << "F_out" << frameNumber << ".txt";
				string str_out = ss_out.str();
				fp_out.open(str_out.c_str(), ios::out|ios::app);

				if(nearest[1]>100 && abs(array1[j][5]-array2[j][5])<0.5*array1[j][5]){
					fp_out << counts_obj << endl;
					for(int i=1;i<8;i++)
							fp_out << array2[j][i] << endl;
					counts_obj++;
				}else{
					fp_out << nearest[0] << endl;
					for(int i=1;i<8;i++)
							fp_out << array2[j][i] << endl;
				}
				//cout<<nearest[0]<<" "<<nearest[1]<<endl;
			}
			

		
		
	}
				
	return 0;
}
