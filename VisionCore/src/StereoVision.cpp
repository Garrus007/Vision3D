#include "StereoVision.h"

StereoVision::StereoVision(std::ostream* stream)
{
	out = stream;
}

/*StereoVision::StereoVision(const char *name)
{

}

StereoVision::StereoVision(StereoCalibData calibData)
{

}*/

/* ���������� ������-������ (���� �����)
 * param[in] left - ����������� � ����� ������ (CV_8UC1 - �����)
 * param[in] right - ����������� � ������ ������
 * param[in] patternSize - ����� ����� ��������� �����
             (����� ��������� �� ������� - 1)
 * result - ����������� ����� ���������� ��� ���������� ������ �����
*/
StereoCalibData StereoVision::Calibrate(const cv::Mat& left, const cv::Mat& right, cv::Size patternSize)
{
	/* �������� ������-����������:
		1. ������� �� ��� ��������� ����� � ���������� ����
		2. ������� �������� ���������� ����� ��������� �����:
		   �������, ��� ��������� ���� ����� ���������� (0,0,0), � ���������
		   ��������� �� ���� �� �������� ���������� - ������ ������
		3. ��������� stereoCalibrate, ����� ���������
		   ������� �����, ��������� � ������� �������� ����� �������� (R, T)
		4. ��������� stereoRectify
		5. �������� ������� ��� remap
	*/

	const int size = 1;

	// ����� �� �����������
	std::vector<cv::Point2f> imagePointsLeftSingle, imagePointsRightSingle;			//������ ��������� �� ����������� �����
	std::vector<std::vector<cv::Point2f>> imagePointsLeft, imagePointsRight;		//������ �������� ����� - �� ������� �� ����������� (� ��� �� ������ �� ������)

	// �������� �����
	std::vector<cv::Point3f> objectPointsSingle;									//�������� ���������� �����
	std::vector<std::vector<cv::Point3f>> objectPoints;								//������ �������� ��������� - �� ������� �� �����������

	//�������� ��������� stereoCalibrate
	cv::Mat CM1(3, 3, CV_64FC1), CM2(3, 3, CV_64FC1);								//CameraMatrix ��������� ��� ������� �� 3D ����� � ������� �� �� 2D ������ � �����������
	cv::Mat D1, D2;		//������������ ���������
	cv::Mat R,			//������� �������� ����� �� ������ � ������ �����
			T,			//������ �������� ����� �� ������ � ������ �����
			E,			//������������ ������� (������������� ����������� ����� ������� �����������?????) (��� ���-�� ������� � ������������ �������)
			F;			//��������������� �������

	//�������� ��������� ��� stereoRectify
	cv::Mat R1, R2,		//������� �������� ��� ����������� (3�3)
			P1, P2;		//����������� ������� � ������������ ������� ��������� (3�4)
						//������ 3 ������� ���� ������ - ����� ������� �����
	cv::Mat Q;			//������� �������� �������� � �������

	//�������� ������ ��� ���������� ���� ������������
	cv::Mat mapLeftX, mapLeftY, mapRightX, mapRightY;

	//1. --------------------------- ���� ��������� ����� �� ����������� --------------------------------------------------------

	//������������: http://goo.gl/kV8Ms1 (������������ ���������������� ������, ������ ��� �������� �������� �������)
	bool isFoundLeft = cv::findChessboardCorners(left, patternSize, imagePointsLeftSingle, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	bool isFoundRight = cv::findChessboardCorners(right, patternSize, imagePointsRightSingle, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

	if (!isFoundLeft || !isFoundRight)
	{
		//���� �� ������� - ������ ����������?
	}

	//�������� ���� (���������� ���������� �� ��������)
	//������������: http://goo.gl/7BjZKd
	cornerSubPix(left, imagePointsLeftSingle, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	cornerSubPix(right, imagePointsRightSingle, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

	//��������� � ������ ��������
	imagePointsLeft.push_back(imagePointsLeftSingle);
	imagePointsRight.push_back(imagePointsRightSingle);


	//���������� ����� - ��� ��������� ����
	cv::drawChessboardCorners(left, patternSize, imagePointsLeftSingle, true);
	cv::drawChessboardCorners(right, patternSize, imagePointsRightSingle, true);

	cv::imwrite("images\\corners_left.png", left);
	cv::imwrite("images\\corners_right.png", right);

	//2. --------------------------- ������ �������� ��������� ����� ------------------------------------------------------------
	/*for (int i = 0; i < patternSize.height; i++)
	{
		for (int j = 0; j < patternSize.width; j++)
		{
			objectPointsSingle.push_back(cv::Point3f(i*size, j*size, 0));
		}
	}*/

	for (int j = 0; j<patternSize.height*patternSize.width; j++)
		objectPointsSingle.push_back(cv::Point3f(j / patternSize.width, j%patternSize.width, 0.0f));
	objectPoints.push_back(objectPointsSingle);

	//3. --------------------------- ������ ���������� --------------------------------------------------------------------------
	//������������: http://goo.gl/mKCH63
	stereoCalibrate(objectPoints, imagePointsLeft, imagePointsRight,
					CM1, D1, CM2, D2, left.size(), R, T, E, F, CV_CALIB_SAME_FOCAL_LENGTH | CV_CALIB_ZERO_TANGENT_DIST,
					cvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5));

	//4. ��������� ������ ��� ������������
	cv::stereoRectify(CM1, D1, CM2, D2, left.size(), R, T, R1, R2, P1, P2, Q);

	//5. ���������� ���� ������������
	cv::initUndistortRectifyMap(CM1, D1, R1, P1, left.size(), CV_32FC1, mapLeftX, mapLeftY);
	cv::initUndistortRectifyMap(CM2, D2, R2, P2, left.size(), CV_32FC1, mapRightX, mapRightY);

	// -------------------------- ���������� ����������� ------------------------------------------------------------------------

	calibData.LeftCameraMatrix = CM1.clone();
	calibData.RightCameraMatrix = CM2.clone();
	calibData.LeftCameraDistortions = D1.clone();
	calibData.RightCameraDistortions = D2.clone();
	calibData.MapLeftX = mapLeftX.clone();
	calibData.MapLeftY = mapLeftY.clone();
	calibData.MapRightX = mapRightX.clone();
	calibData.MapRightY = mapRightY.clone();
	calibData.Q = Q;

	/*(*out) << "CM1:\n";
	StaticHelpers::printMatrixStream<double>(CM1, *out);

	(*out) << "\nCM2:\n";
	StaticHelpers::printMatrixStream<double>(CM2, *out);

	(*out) << "\nD1:\n";
	StaticHelpers::printMatrixStream<double>(D1, *out);

	(*out) << "\nD2:\n";
	StaticHelpers::printMatrixStream<double>(D2, *out);

	(*out) << "\nR:\n";
	StaticHelpers::printMatrixStream<double>(R, *out);

	(*out) << "\nT:\n";
	StaticHelpers::printMatrixStream<double>(T, *out);

	(*out) << "\nE:\n";
	StaticHelpers::printMatrixStream<double>(E, *out);

	(*out) << "\nF:\n";
	StaticHelpers::printMatrixStream<double>(F, *out);

	(*out) << "\nP1:\n";
	StaticHelpers::printMatrixStream<double>(P1, *out);

	(*out) << "\nR1:\n";
	StaticHelpers::printMatrixStream<double>(R2, *out);

	(*out) << "\nR2:\n";
	StaticHelpers::printMatrixStream<double>(R2, *out);

	(*out) << "\nP1:\n";
	StaticHelpers::printMatrixStream<double>(P1, *out);

	(*out) << "\nP2:\n";
	StaticHelpers::printMatrixStream<double>(P2, *out);

	(*out) << "\nQ:\n";
	StaticHelpers::printMatrixStream<double>(Q, *out);*/

	this->calibData = calibData;
	return StereoCalibData();
}


/* ���������� ������ ����� �� ���� ������������
 * param[in] left - ����� ����������� � ��������������� ������
 * param[in] right - ������ ����������� � ��������������� ������
 * result - ������ �����
 */
IPointCloudStorage* StereoVision::CalculatePointCloud(const cv::Mat& left, const cv::Mat& right) const
{
	cv::Mat leftRemaped, rightRemaped;
	cv::remap(left, leftRemaped, calibData.MapLeftX, calibData.MapLeftY, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());
	cv::remap(right, rightRemaped, calibData.MapRightX, calibData.MapRightY, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());

	cv::imwrite("images\\left_remap.png", leftRemaped);
	cv::imwrite("images\\right_remap.png", rightRemaped);


	return NULL;
}

StereoCalibData StereoVision::GetCalibData()
{
	return StereoCalibData();
}