#include <iostream>
#include <assert.h>

#include <riegl/scanifc.h>

#include <lvr2/io/PointBuffer.hpp>
#include <lvr2/io/Model.hpp>

#include <lvr2/io/RxpIO.hpp>

namespace lvr2
{


ModelPtr RxpIO::read(std::string filename) {
    return read(filename, 1, Matrix4<Vec>());
}

ModelPtr RxpIO::read(std::string filename, int reduction_factor, Matrix4<Vec> tf) {

    if (reduction_factor < 1) {
        reduction_factor = 1;
    }

    // open file
    point3dstream_handle stream;
    if (check_error(scanifc_point3dstream_open(("file:" + filename).c_str(), 0, &stream))) {
        return ModelPtr();
    }

    std::vector<Vector<Vec>> data;
    const unsigned int point_buf_size = 32768;
    scanifc_xyz32_t point_buf[point_buf_size];
    int end_of_frame = 0;
    unsigned int got = 0;
    int count = 0;
    Vector<Vec> cur_point;

    // read data
    do {

        if (check_error(scanifc_point3dstream_read(stream, point_buf_size, point_buf, 0, 0, &got, &end_of_frame))) {
            return ModelPtr();
        }

        // use points here
        for (int i = 0; i < got; i++) {
            if (count++ % reduction_factor == 0) {
                cur_point.x = point_buf[i].x;
                cur_point.y = point_buf[i].y;
                cur_point.z = point_buf[i].z;

                data.push_back(tf * cur_point);
            }
        }

    } while(end_of_frame || got);


    // TODO check this for null?
    float *data2 = new float[data.size() * 3];
    assert(data2 =! NULL);

    // TODO Since we don't know the size in advance there is no better way?
    for (int i = 0; i < data.size(); i++) {
        data2[i*3 + 0] = data[i].x;
        data2[i*3 + 1] = data[i].y;
        data2[i*3 + 2] = data[i].z;
    }

    PointBufferPtr pbp(new PointBuffer());
    pbp->setPointArray(floatArr(data2), data.size());

    // close file
    if (check_error(scanifc_point3dstream_close(stream))) {
        // actually not so bad?
    }

    return ModelPtr(new Model(MeshBufferPtr(), pbp));
}

void RxpIO::save(std::string filename) {
    std::cout << "[RxpIO] Error: Saving files to .rxp format is not supported" << std::endl;
}

bool RxpIO::check_error(int error_code) {
    if (error_code) {

        // TODO check if string is zero delimited IF msg_size > (>=) msg_buf_size!
        unsigned int msg_buf_size = 2048;
        char msg_buf[msg_buf_size];
        unsigned int msg_size = 0;

        scanifc_get_last_error(msg_buf, msg_buf_size, &msg_size);

        std::cout << "[RxpIO] Error: " << msg_buf << std::endl;

        return true;

    } else {
        return false;
    }
}

} // namespace lvr2
