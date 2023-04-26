// part1.cpp
//
// This SYCL program should create a parallel (vectorized) version of the following
// sequential code:
//
// for (int i=0; i < VECTOR_SIZE; i++)
//   z[i] = x[i] + y[i];
//
// TODO: The provided code has some errors and missing code. Some of these will cause
// compiler errors. Some will cause runtime errors. You need to fix them an ensure that
// the output tests pass (you will see SUCCESS!) in the terminal.

#include <iostream>
#include <vector>
#include <random>

#include <CL/sycl.hpp>

const int NUM_INPUTS = 100000;

class vector_add;

int main(int argc, char* argv[]) { 
  
  std::vector<int> x_h(NUM_INPUTS);
  std::vector<int> y_h(NUM_INPUTS);
  std::vector<int> z_h(NUM_INPUTS);
  std::vector<int> correct_out(NUM_INPUTS);

  // Use C++11 randomization for input
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(-100, 100);

  for (size_t i=0; i < NUM_INPUTS; i++) {
    x_h[i] = dist(gen);
    y_h[i] = dist(gen);
    z_h[i] = 0;

    // Calculate correct outputs for comparison.
    correct_out[i] = x_h[i] + y_h[i];
  }
  

  try {
    cl::sycl::queue queue(cl::sycl::default_selector_v);
    
    cl::sycl::buffer<int, 1> x_buf {x_h.data(), cl::sycl::range<1>(x_h.size()) };
    cl::sycl::buffer<int, 1> y_buf {y_h.data(), cl::sycl::range<1>(y_h.size()) };
    cl::sycl::buffer<int, 1> z_buf {z_h.data(), cl::sycl::range<1>(z_h.size()) };
    
    queue.submit([&](cl::sycl::handler& handler) {

      cl::sycl::accessor x_d(x_buf, handler, cl::sycl::read_only);
      cl::sycl::accessor y_d(y_buf, handler, cl::sycl::write_only);
      cl::sycl::accessor z_d(z_buf, handler, cl::sycl::write_only);

      handler.parallel_for<class vector_add>(cl::sycl::range<1> { NUM_INPUTS }, [=](cl::sycl::id<1> i) {
          z_d[i] = x_d[i] + y_d[i];
        });

      });

    queue.wait();

    // Check for correctness.
    if (z_h == correct_out) {
      std::cout << "SUCCESS!" << std::endl;
    }
    else {
      std::cout << "ERROR: Execution failed." << std::endl;
    }    
  }
  catch (cl::sycl::exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}

// #include <CL/sycl.hpp>

// #include <iostream>
// #include <vector>
// #include <random>

// const int NUM_INPUTS = 100000;

// class vector_add;

// int main(int argc, char* argv[]) { 

//   std::vector<int> x_h(NUM_INPUTS);
//   std::vector<int> y_h(NUM_INPUTS);
//   std::vector<int> z_h(NUM_INPUTS);
//   std::vector<int> correct_out(NUM_INPUTS);

//   std::random_device rd;
//   std::mt19937 gen(rd());
//   std::uniform_int_distribution<> dist(-100, 100);

//   for (size_t i=0; i < NUM_INPUTS; i++) {
//     x_h[i] = dist(gen);;
//     y_h[i] = dist(gen);;
//     z_h[i] = 0;
//     //correct_out[i] = i + i;
//     correct_out[i] = x_h[i] + y_h[i];
//   }

//   try {
      
//     cl::sycl::queue queue(cl::sycl::default_selector_v);

//     // CHANGES FROM PREVIOUS EXAMPLE
//     // To fix the bug from the previous version where the output was always 0,
//     // we can add new scope for all the device code. It will be explained in detail later,
//     // but the previous problem was that the output was never transferred from device memory
//     // back to the host. By putting the device code in a separate scope, all objects are
//     // "destructed" at the end of the scope, which for out_buf causes the host to transfer
//     // the outputs back from the device. There are other methods we could have used,
//     // but using a separate scope is common, especially since that scope is usually created
//     // by a try-catch, as we will see in the next example.
//     {
//       cl::sycl::buffer<int, 1> x_buf {x_h.data(), cl::sycl::range<1>(x_h.size()) };
//       cl::sycl::buffer<int, 1> y_buf {y_h.data(), cl::sycl::range<1>(y_h.size()) };
//       cl::sycl::buffer<int, 1> z_buf {z_h.data(), cl::sycl::range<1>(z_h.size()) };
      


//       queue.submit([&](cl::sycl::handler& handler) {
    
//     cl::sycl::accessor x_d(x_buf, handler, cl::sycl::read_only);
//     cl::sycl::accessor y_d(y_buf, handler, cl::sycl::read_only);
//     cl::sycl::accessor z_d(z_buf, handler, cl::sycl::write_only);
    
//     handler.parallel_for<class vector_add>(cl::sycl::range<1> { NUM_INPUTS }, [=](cl::sycl::id<1> i) {
//         z_d[i] = x_d[i] + y_d[i];
//         //z_d[i] = x_h[i] + y_h[i]; error
//       });
    
//         });
      
//       queue.wait();
      
//     }
    
//     // CHANGES FROM PREVIOUS EXAMPLE
//     // As an alternative to adding a new scope to ensure the output is transferred
//     // back to the host, we can explicitly transfer the outputs with the following
//     // code, where the host requests read access to the output buffer.
//     // 
//     // out_buf.get_access<cl::sycl::access::mode::read>();
      
//     // std::cout << "Operation complete:\n"
//     //     << "[" << x_h[0] << "] + [" << y_h[0] << "] = [" << z_h[0] << "]\n"
//     //     << "[" << x_h[1] << "] + [" << y_h[1] << "] = [" << z_h[1] << "]\n"
//     //     << "...\n"
//     //     << "[" << x_h[NUM_INPUTS - 1] << "] + [" << y_h[NUM_INPUTS - 1] << "] = [" << z_h[NUM_INPUTS - 1] << "]\n"
//     //     << std::endl;

//     if (z_h == correct_out) {
//       std::cout << "SUCCESS!" << std::endl;
//     }
//     else {
//       std::cout << "ERROR: Execution failed." << std::endl;
//     }
//     }
//     catch (cl::sycl::exception& e) {
//       std::cout << e.what() << std::endl;
//       return 1;
//     }

//   return 0;
// }