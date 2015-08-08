#include "GridStage.hpp"

// default constructor
GridStage::GridStage(double voxel_size) : AbstractStage()
{
	m_inQueue = boost::shared_ptr<BlockingQueue>(
			new BlockingQueue());
	m_outQueue = boost::shared_ptr<BlockingQueue>(
			new BlockingQueue());
	grid_count_ = 0;
	voxel_size_  = voxel_size;
	bbox_ = BoundingBox<cVertex>(0.0, 0.0, 0.0, 300.0, 300.0, 300.0);
	bbox_.expand(300.0, 300.0, 300.0);
}

void GridStage::FirstStep() { /* omit */ };

void GridStage::Step()
{
	auto cloud_work = boost::any_cast<pair<cv::Mat&, Vec3i> >(getInQueue()->Take());
	cv::Mat& cloud = cloud_work.first;
	Vec3i offset = cloud_work.second;
	ScopeTime* grid_time = new ScopeTime("Grid Creation");
	Point* tsdf_ptr = cloud.ptr<Point>();				
	TGrid* act_grid = NULL;
	if(last_grid_queue_.size() == 0)
		act_grid = new TGrid(voxel_size_, bbox_, tsdf_ptr, cloud.cols, offset[0], offset[1], offset[2], NULL, true);
	else
		act_grid = new TGrid(voxel_size_, bbox_, tsdf_ptr, cloud.cols, offset[0], offset[1], offset[2], last_grid_queue_.front(), true);
	grid_queue_.push(act_grid);
	std::cout << "    ####     1 Finished grid number: " << grid_count_ << "   ####" << std::endl;
	//grid_ptr->saveGrid("./slices/grid" + std::to_string(slice_count_) + ".grid");
	double recon_factor = (grid_time->getTime()/cloud.cols) * 1000;
	timeStats_.push_back(recon_factor);
	delete grid_time;
	if(last_grid_queue_.size() > 0)
	{
		delete last_grid_queue_.front();
		last_grid_queue_.pop();
	}
	last_grid_queue_.push(act_grid);
	getOutQueue()->Add(act_grid);
	grid_count_++;
	if(last_shift_ && getInQueue()->size() == 0)
		done(true);
}
void GridStage::LastStep()	{ /* omit */ };
