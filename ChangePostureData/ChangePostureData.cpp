// ChangePostureData.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>


int main()
{
	//txtPostureData読み込み
	std::ifstream ifs("GetPositionALL.txt");
	std::string str;
	int CountData = 0;
	//postureDataを入れるvector確保
	//std::vector<std::vector<double>>txtPosture(3, std::vector<double>(25));
	std::vector<std::vector<std::vector<double>>> Posture(3, std::vector<std::vector<double>>(25, std::vector<double>(3000, 0)));//1595

	std::vector< std::vector<double> > position( 25, std::vector<double>( 3 ) );
	std::vector< std::vector<double> > PositionBase( 25, std::vector<double>( 3 ) );
	std::vector< std::vector<double> > RotatePosition( 25, std::vector<double>( 3 ) );
	std::vector< std::vector<double> > XRotatePosition( 25, std::vector<double>( 3 ) );
	std::vector< std::vector<double> > YRotatePosition( 25, std::vector<double>( 3 ) );
	std::vector< std::vector<double> > ZRotatePosition( 25, std::vector<double>( 3 ) );

	std::ofstream ofs( "GetPositionALL.txt", std::ios::out | std::ios::app );
	
	std::ofstream ofs2( "GetPositionArm.txt", std::ios::out | std::ios::app );

	while (getline(ifs, str))
	{
		std::string tmp;
		std::istringstream stream(str);
		int CountXYZ = 0, CountPosture = 0;
		while (getline(stream, tmp, '\t'))
		{
			//文字列から数字(double)に変換
			std::istringstream is;
			is.str(tmp);    
			double x;             
			is >> x;                
			//postureにtxtPostureDataを代入
			Posture[CountXYZ][CountPosture][CountData] = x;
			//std::cout <<"("<<CountXYZ<<":"<<CountPosture<<":"<<CountData<<")->"<< x << std::endl;
			
			//countを取ってpostureに代入
			CountXYZ++;
			if (CountXYZ == 3)
			{
				CountPosture++;
				CountXYZ = 0;
			}
			if (CountPosture == 25)
			{
				CountData++;
				CountPosture = 0;
			}
		}
	}

	for ( int i = 0; i <= CountData; i++ )
	{
		//テキストベースでpositionを保存
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				Posture[PosCount][count][i] = position[count][PosCount];
			}
		}
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				PositionBase[count][PosCount] = position[count][PosCount] - position[0][PosCount];
			}
		}


		//座標変換ユーザの角度
		double crossX = ( PositionBase[8][1] * PositionBase[4][2] ) - ( PositionBase[8][2] * PositionBase[4][1] );
		double crossY = ( PositionBase[8][2] * PositionBase[4][0] ) - ( PositionBase[8][0] * PositionBase[4][2] );
		double crossZ = ( PositionBase[8][0] * PositionBase[4][1] ) - ( PositionBase[8][1] * PositionBase[4][0] );//外積
		double nLength = sqrtf( ( crossX*crossX ) + ( crossY*crossY ) + ( crossZ*crossZ ) );//normalize

		double nx = crossX / nLength;//1より大きかったらだめifブンツクレ
		double ny = crossY / nLength;
		double nz = crossZ / nLength;//面法線ベクトル
		double nxx = crossX / sqrtf( ( crossX*crossX ) + ( crossY*crossY ) );
		double nzz = crossZ / sqrtf( ( crossX*crossX ) + ( crossZ*crossZ ) );
		/*
		if ( nx >= 1 )nx = 1;
		if ( ny >= 1 )ny = 1;
		if ( nz >= 1 )nz = 1;
		*/
		double cosX = nx;//nx / nLength;
		double cosY = ny;//ny / nLength;
		double cosZ = nz;//nz / nLength;//面の傾き(cos)？
		double cosXX = nxx;//x / √x^2+y^2
		double cosZZ = nzz;
		double cosZZZ = ( PositionBase[4][1] ) / sqrtf( ( PositionBase[4][1] * PositionBase[4][1] ) + ( PositionBase[4][2] * PositionBase[4][2] ) );

		double sinX = crossZ / nLength;//sqrtf( 1 - ( cosX*cosX ) );//マイナスになってる?正じゃないといけない
		double sinY = sqrtf( ( crossX*crossX ) + ( crossY*crossY ) ) / nLength;//sqrtf( 1 - ( cosY*cosY ) );
		double sinZ = crossY / nLength;//sqrtf( 1 - ( cosZ*cosZ ) );//面の傾き(sin)?
		double sinXX = sqrtf( 1 - ( cosXX*cosXX ) );
		double sinZZ = crossX / sqrtf( ( crossX*crossX ) + ( crossZ*crossZ ) );
		double sinZZZ = ( PositionBase[4][2] ) / sqrtf( ( PositionBase[4][1] * PositionBase[4][1] ) + ( PositionBase[4][2] * PositionBase[4][2] ) );
		
		//各軸回転行列
		std::vector< std::vector<double> > vectorRx( 3, std::vector<double>( 3 ) );
		std::vector< std::vector<double> > vectorRy( 3, std::vector<double>( 3 ) );
		std::vector< std::vector<double> > vectorRz( 3, std::vector<double>( 3 ) );
		std::vector< std::vector<double> > vectorR( 3, std::vector<double>( 3 ) );

		vectorRx[0][0] = 1;
		vectorRx[0][1] = 0;
		vectorRx[0][2] = 0;
		vectorRx[1][0] = 0;
		vectorRx[1][1] = sinY;//cosX(90"-")
		vectorRx[1][2] = -cosY;//-sinX
		vectorRx[2][0] = 0;
		vectorRx[2][1] = cosY;//sinX
		vectorRx[2][2] = sinY;//cosX//X軸周り回転行列

		vectorRy[0][0] = cosZZ;//cosZ;//cosY
		vectorRy[0][1] = 0;
		vectorRy[0][2] = -sinZZ;//sinZ;//sinY
		vectorRy[1][0] = 0;
		vectorRy[1][1] = 1;
		vectorRy[1][2] = 0;
		vectorRy[2][0] = sinZZ;//-sinZ;//sinY
		vectorRy[2][1] = 0;
		vectorRy[2][2] = cosZZ;//cosY//Y軸周り回転行列

		vectorRz[0][0] = cosX;//cosZ
		vectorRz[0][1] = -sinX;//-sinZ
		vectorRz[0][2] = 0;
		vectorRz[1][0] = sinX;//sinZ
		vectorRz[1][1] = cosX;//cosZ
		vectorRz[1][2] = 0;
		vectorRz[2][0] = 0;
		vectorRz[2][1] = 0;
		vectorRz[2][2] = 1;//Z軸周り回転行列


		//行列計算->Y軸周りの計算
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				//cout << position[count][PosCount] << endl;
				//ofs << position[count][PosCount] << "\t";
				if ( PosCount == 0 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						YRotatePosition[count][PosCount] += PositionBase[count][i] * vectorRy[0][i];
					}
				}
				else if ( PosCount == 1 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						YRotatePosition[count][PosCount] += PositionBase[count][i] * vectorRy[1][i];
					}
				}
				else if ( PosCount == 2 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						YRotatePosition[count][PosCount] += PositionBase[count][i] * vectorRy[2][i];
					}
				}
				//RotatePosition[count][PosCount] = PositionBase[count][PosCount];
			}
		}

		//行列計算->X軸周りの計算
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				if ( PosCount == 0 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						XRotatePosition[count][PosCount] += YRotatePosition[count][i] * vectorRx[0][i];
					}
				}
				else if ( PosCount == 1 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						XRotatePosition[count][PosCount] += YRotatePosition[count][i] * vectorRx[1][i];
					}
				}
				else if ( PosCount == 2 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						XRotatePosition[count][PosCount] += YRotatePosition[count][i] * vectorRx[2][i];
					}
				}
			}
		}


		std::vector<double> shoulder( 3 );
		shoulder[0] = /*PositionBase[4][0] - PositionBase[8][0];*/XRotatePosition[4][0] - XRotatePosition[8][0];
		shoulder[1] = /*PositionBase[4][1] - PositionBase[8][1]; */XRotatePosition[4][1] - XRotatePosition[8][1];
		shoulder[2] = /*PositionBase[4][2] - PositionBase[8][2];*/XRotatePosition[4][2] - XRotatePosition[8][2];

		double cosXshoulder = shoulder[0] / sqrtf( ( shoulder[0] * shoulder[0] ) + ( shoulder[1] * shoulder[1] ) + ( shoulder[2] * shoulder[2] ) );
		double sinXshoulder = shoulder[1] / sqrtf( ( shoulder[0] * shoulder[0] ) + ( shoulder[1] * shoulder[1] ) + ( shoulder[2] * shoulder[2] ) );
		vectorRz[0][0] = cosXshoulder;//cosZ
		vectorRz[0][1] = sinXshoulder;//-sinZ
		vectorRz[0][2] = 0;
		vectorRz[1][0] = -sinXshoulder;//sinZ
		vectorRz[1][1] = cosXshoulder;//cosZ
		vectorRz[1][2] = 0;
		vectorRz[2][0] = 0;
		vectorRz[2][1] = 0;
		vectorRz[2][2] = 1;//Z軸周り回転行列

		//行列計算->Z軸周りの計算
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				if ( PosCount == 0 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						ZRotatePosition[count][PosCount] += XRotatePosition[count][i] * vectorRz[0][i];
					}
				}
				else if ( PosCount == 1 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						ZRotatePosition[count][PosCount] += XRotatePosition[count][i] * vectorRz[1][i];
					}
				}
				else if ( PosCount == 2 )
				{
					for ( int i = 0; i <= 2; i++ )
					{
						ZRotatePosition[count][PosCount] += XRotatePosition[count][i] * vectorRz[2][i];
					}
				}
			}
		}
		//今回はz軸回転->y軸回転->z軸回転順に回転行列を計算した原点は骨格の腰中央点
		RotatePosition = ZRotatePosition;
		//どの回転結果を表示するか->以下RotatePositionを表示


		//座標変換後のpostureをテキストファイルに保存
		std::ofstream ofs1( "RotatePostureALL.txt", std::ios::out | std::ios::app );
		for ( int count = 0; count <= 24; count++ )
		{
			for ( int PosCount = 0; PosCount <= 2; PosCount++ )
			{
				//cout << position[count][PosCount] << endl;
				ofs1 << RotatePosition[count][PosCount] * 80000 << "\t";

			}
			if ( count == 24 )
			{
				ofs1 << "\n";
			}
		}
		std::ofstream ofs3( "RotatePostureArm.txt", std::ios::out | std::ios::app );
		for ( int count = 0; count <= 24; count++ )
		{
			if ( count == 8 || count == 9 || count == 11 )
			{
				ofs3 << RotatePosition[count][0] * 80000 << "\t" << -RotatePosition[count][1] * 80000 << "\t" << RotatePosition[count][2] * 80000 << "\t";
			}
			else
			{
				ofs3 << RotatePosition[count][0] * 80000 << "\t" << -RotatePosition[count][1] * 80000 << "\t" << RotatePosition[count][2] * 80000 << "\t";
			}
			if ( count == 24 )
			{
				ofs3 << "\n";
			}
		}

	}


    return 0;
}

