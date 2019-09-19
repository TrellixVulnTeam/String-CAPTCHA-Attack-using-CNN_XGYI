#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int GetPixelData(Mat image, uchar* scale, int j, int i);

int pixel_threshold = 120;
int min_size = 180;

int main() {
// ���� �̹��� ���ϸ�
	int imagename;
	FILE *fp = fopen("remain.txt", "w");
	
	for (imagename = 0; imagename < 100;imagename++) {
		char LOC[100] = "C:/Users/lawle/Downloads/������ũ_eval/"; // �̹��� ���� ���
		char NUM[20] = "", buffer[2], NUM2[20] = "";

		int i;

		sprintf(buffer, "%d", 0);
		sprintf(NUM2, "%d", imagename);

		for (i = 0; i < (4 - strlen(NUM2)); i++) {
			strcat(NUM, buffer);
		}
		strcat(NUM, NUM2);

		printf("%s ", NUM);

		strcat(LOC, NUM);
		strcat(LOC, ".png");

		Mat image = imread(LOC, CV_LOAD_IMAGE_GRAYSCALE); // ���� �̹����� GRAYSCALE�� �ҷ�����

		if (image.empty()) {
			std::cout << "������ �ҷ��� �� �����ϴ�." << endl;
			return -1;
		}

		uchar *scale;
		int j, check = 0;
		int sRow, sCol, bRow, bCol;
		/* sRow, sCol : ����� �����ϴ� �κ��� ���� �� ������ ��ǥ
		bRow, bCol : ����� �����ϴ� �κ��� ������ �Ʒ� ������ ��ǥ*/

		int nRow = image.rows;
		int nCol = image.cols;

		// ����� �����ϴ� �κ� �ڸ��� : �ȼ� ���� 200 ������ �κ��� ������� �Ǵ�.
		for (i = 0; i < nCol; i++) {
			for (j = 0; j < nRow; j++) {
				scale = nullptr;
				if (GetPixelData(image, scale, j, i) < 200) {
					sRow = j;
					sCol = i;
					check = 1;
					break;
				}
			}
			if (check == 1) break;
		}

		check = 0;

		for (j = nRow - 2; j >= 0; j--) {
			for (i = nCol - 2; i >= 0; i--) {
				scale = nullptr;
				if (GetPixelData(image, scale, j, i) < 200) {
					bRow = j;
					bCol = i;
					check = 1;
					break;
				}
			}
			if (check == 1)	break;
		}


		Rect rect(sCol, sRow, bCol - sCol, bRow - sRow);
		Mat subimage = image(rect); // ����� �����ϴ� �κ� = subimage�� ����.

		nRow = subimage.rows;
		nCol = subimage.cols;

		// GRAYSCALE -> Black/White�� ��ȯ. Threshold = 120.
		for (i = 0; i < nCol; i++) {
			for (j = 0; j < nRow; j++) {
				scale = nullptr;
				if (GetPixelData(subimage, scale, j, i) < pixel_threshold) subimage.at<uchar>(j, i) = 255;
				else subimage.at<uchar>(j, i) = 0;

			}
		}
		
		// ���� ���� : �����ڸ� ���� 23 �ȼ� ������ ���ڰ� ���ٴ� Ư¡.
		for (i = 0; i < nCol; i++) {
			for (j = 0; j < nRow; j++) {
				scale = nullptr;
				if (i < 23 || j < 23 || i >(nCol - 23) || j >(nRow - 23)) subimage.at<uchar>(j, i) = 255;
			}
		}


		// Morphology ���� : CLOSE and OPEN
		Mat CLOSE, OPEN;
		Mat mask = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));

		morphologyEx(subimage, CLOSE, MORPH_CLOSE, mask);
		morphologyEx(CLOSE, OPEN, MORPH_OPEN, mask);

		imshow("r??", OPEN);
		waitKey(0);

		// Contour detection	
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours(OPEN, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point());

		vector<vector<Point>> contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Rect> boundRect2(contours.size());
		vector<Rect> boundRect3(contours.size());

		int refinery_count = 0;

		for (int i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));

			if (boundRect[i].area() >= min_size && boundRect[i].area() <= 1200) {
				boundRect2[refinery_count] = boundRect[i];
				refinery_count++;
			}
		}

		boundRect2.resize(refinery_count);
		boundRect3.resize(refinery_count);

		printf("%d", refinery_count);

		if (refinery_count != 6) {
			printf(": %s\n", NUM); fprintf(fp, "%d, ", imagename);
		}
		else printf("\n");

		Mat Letter[6];
		int min, min_last=0, count;

		for (i = 0; i < 6; i++) {
			min = 9999;
			count = 0;
			for (j = 0; j < 6; j++) {
				if (boundRect2[j].tl().x < min && boundRect2[j].tl().x > min_last) {
					count = j;
					min = boundRect2[j].tl().x;
				}
			}
			boundRect3[i] = boundRect2[count];
			min_last = boundRect2[count].tl().x;
		}

		for (i = 0; i < 6; i++) boundRect2[i] = boundRect3[i];


		for (i = 0; i < 6; i++) {
			Rect rectangle(boundRect2[i].tl().x, boundRect2[i].tl().y, boundRect2[i].br().x - boundRect2[i].tl().x, boundRect2[i].br().y - boundRect2[i].tl().y);
			Letter[i] = (Mat)OPEN(rectangle);
		}

		// ���� ���� �̹��� ����� 32*32�� ����
		for (i = 0; i < 6; i++) resize(Letter[i], Letter[i], Size(32, 32));

		// ���� ���� 6�� �̹��� ����.
		char filename1[20] = "img";
		strcat_s(filename1, NUM);
		strcat_s(filename1, "_1.jpg");
		imwrite(filename1, Letter[0]);

		char filename2[20] = "img";
		strcat_s(filename2, NUM);
		strcat_s(filename2, "_2.jpg");
		imwrite(filename2, Letter[1]);

		char filename3[20] = "img";
		strcat_s(filename3, NUM);
		strcat_s(filename3, "_3.jpg");
		imwrite(filename3, Letter[2]);

		char filename4[20] = "img";
		strcat_s(filename4, NUM);
		strcat_s(filename4, "_4.jpg");
		imwrite(filename4, Letter[3]);

		char filename5[20] = "img";
		strcat_s(filename5, NUM);
		strcat_s(filename5, "_5.jpg");
		imwrite(filename5, Letter[4]);

		char filename6[20] = "img";
		strcat_s(filename6, NUM);
		strcat_s(filename6, "_6.jpg");
		imwrite(filename6, Letter[5]);

	}
}

// Ư�� ��ġ�� �ȼ� ���� �ҷ����� �Լ�.
int GetPixelData(Mat image, uchar* scale, int j, int i) {
	uchar *data = image.data;

	int nRow = image.rows;
	int nCol = image.cols;

	if (i > nCol || j > nRow) {
		cout << "Selected pixel is out of range." << endl;
		return -1;
	}

	scale = image.data + j * image.step + i * image.elemSize() + 0;

	return (int)*scale;
}
