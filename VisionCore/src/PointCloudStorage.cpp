#include "PointCloudStorage.h"

/* ������� ������ ����� �� ������ ������ c 3��������
* ������������ ����� � ������� � �����
* param[in] cloud - ������ �����, ��������� reprojectImageTo3d
* param[in] colors - ���������� � ����� �����
*/
PointCloudStorage::PointCloudStorage(cv::Mat cloud, cv::Mat colors)
{
	_getPointsFromMat(cloud, colors);
}

/* ������� ������ ����� �� ������ ������ c 3��������
* ������������ �����
* param[in] cloud - ������ �����, ��������� reprojectImageTo3d
* param[in] colors - ���������� � ����� �����
*/
PointCloudStorage::PointCloudStorage(cv::Mat cloud)
{
	_getPointsFromMat(cloud);
}

//������� ������ ����� �� ������ ��������� � �����
//������������ ���������� �����
void PointCloudStorage::_getPointsFromMat(cv::Mat cloud, cv::Mat colors)
{
	bool isColorExists = (cloud.rows == colors.rows) && (cloud.cols == colors.cols);
	cv::Scalar color(0, 0, 0);

	for (int x = 0; x < cloud.cols; x++)
	{
		for (int y = 0; y < cloud.rows; y++)
		{
			if (isColorExists)
				color = colors.at<cv::Scalar>(y, x);

			auto point = cloud.at<cv::Vec3f>(y, x);
			if (isinf(point[0]) || isinf(point[1]) || isinf(point[2]))continue;

			_points.push_back(ColoredPoint3d(point[0], point[1], point[2], color));
		}
	}
}

//��������� ��� � ���� obj
void PointCloudStorage::SaveToObj(const char* filename) const
{
	std::ofstream stream(filename);
	for (int i = 0; i < _points.size(); i++)
	{
		auto point = _points[i];
		stream << "v " << point.X <<" "<< point.Y << " " << point.Z << "\n";
	}
	stream.close();
}