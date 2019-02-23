// raytracingBook.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>

//for multiprocessing
#include <string>
#include <thread>
#include <vector>
//#include <mutex>
//#include <algorithm>
#include <future>
//#include <iterator>
#include <chrono>

//#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"


//std::default_random_engine reng;
//std::mt19937 mt(19);
//std::uniform_real_distribution<float> uni_dist(0.0f, 1.0f);

// replace rand()
//vec3 random_in_unit_sphere() {
//	vec3 p;
//	do {
//		p = 2.0*vec3(uni_dist(mt), uni_dist(mt), uni_dist(mt)) - vec3(1, 1, 1);
//		//p = 2.0*vec3((rand() % 100 / float(100)), (rand() % 100 / float(100)), (rand() % 100 / float(100))) - vec3(1, 1, 1);
//	} while (p.squared_length() >= 1.0);
//	return p;
//}

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
		//vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		//return 0.5*color(ray(rec.p, target - rec.p), world);
		// return the normal as surface color
		//return 0.5*vec3(rec.normal.x()+1, rec.normal.y()+1, rec.normal.z()+1);
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

hitable *random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = uni_dist(mt);
			vec3 center(a + 0.9*uni_dist(mt), 0.2, b + 0.9*uni_dist(mt));
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(uni_dist(mt)*uni_dist(mt), uni_dist(mt)*uni_dist(mt), uni_dist(mt)*uni_dist(mt))));
				}
				else if (choose_mat < 0.95) {
					list[i++] = new sphere(center, 0.2, new metal(vec3(0.5*(1 + uni_dist(mt)), 0.5*(1 + uni_dist(mt)), 0.5*(1 + uni_dist(mt))), 0.5*uni_dist(mt)));
				}
				else {
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}
	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	return new hitable_list(list, i);
}

// int& is reference, int is copy
std::string render_loop_test(int nx, int ny, int y_start, int y_end, int ns, hitable *world, camera cam) {
	std::string tile_data = "";
	for (int j = y_end - 1; j >= y_start; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + uni_dist(mt)) / float(nx);
				float v = float(j + uni_dist(mt)) / float(ny);
				ray r = cam.get_ray(u, v);
				// vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);
			tile_data += std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + "\n";
		}
	}
	return tile_data;
}


void render_loop(int& nx, int& y_start, int& ny, int& ns, hitable *world, camera& cam) {
	std::ofstream myfile("test2.ppm");
	if (myfile.is_open()) {
		myfile << "P3\n" << nx << " " << ny << "\n255\n";

		for (int j = ny - 1; j >= 0; j--) {
			for (int i = 0; i < nx; i++) {
				vec3 col(0, 0, 0);
				for (int s = 0; s < ns; s++) {
					float u = float(i + uni_dist(mt)) / float(nx);
					float v = float(j + uni_dist(mt)) / float(ny);
					ray r = cam.get_ray(u, v);
					// vec3 p = r.point_at_parameter(2.0);
					col += color(r, world, 0);
				}
				col /= float(ns);
				col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
				int ir = int(255.99*col[0]);
				int ig = int(255.99*col[1]);
				int ib = int(255.99*col[2]);
				myfile << ir << " " << ig << " " << ib << std::endl;
			}
		}
		myfile.close();
	}
	else std::cout << "Unable to open file";
}

std::string fetchString(std::string recvdData, int thread_seed)
{
	// Make sure that function takes 5 seconds to complete
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	std::string s = "";
	srand(thread_seed);
	for (int i = 0; i < 10; i++) {
		s += std::to_string(rand() % 256) + " " + std::to_string(rand() % 256) + " " + std::to_string(rand() % 256) + "\n";
	}
	//Do stuff like creating DB Connection and fetching Data
	return "DB_" + recvdData + "\n" + s;
}

int main()
{
	int nx = 600;
	int ny = 300;
	int ns = 4;
	std::string filename = "test3.ppm";

	// fetch from max_threads number
	int total_threads = std::thread::hardware_concurrency();
	int max_threads = (total_threads - 1)*4;
	std::cout << "available system threads: " << total_threads;
	std::cout << "used system threads: " << max_threads;

	int total_tiles = 60;
	int tile_loops = total_threads / max_threads;

	int ny_tile = ny / max_threads;

	//hitable *list[5];
	//list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	//list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	//list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.2));
	//list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	//list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	//hitable *world = new hitable_list(list, 5);

	hitable *world = random_scene();

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	//float dist_to_focus = (lookfrom - lookat).length();
	float dist_to_focus = 10.0;
	float aperature = 0.1;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperature, dist_to_focus);

	//// start time for render loop
	//size_t start = time(NULL);

	////int ystart = 0;
	//render_loop(nx, ystart, ny, ns, world, cam);

	//printf("**MyProgram::time elapsed= %lds\n", time(NULL) - start);

	//--------------------------------
	// thread with string return

	// time using chrono
	auto start_timer = std::chrono::steady_clock::now();

	std::ofstream myfile(filename);
	if (myfile.is_open()) {
		myfile << "P3\n" << nx << " " << ny << "\n255\n";

		std::vector<std::future<std::string>> futures_vector;
		for (int i = max_threads-1; i >= 0; i--) {
			int y_start = i * ny_tile;
			int y_end = y_start + ny_tile;
			std::cout << y_start << " " << y_end << std::endl;
			//futures_vector.push_back(std::async(std::launch::async, fetchString, "Data", i));
			//futures_vector.push_back(std::async(std::launch::async, render_loop_test, nx, ystart, ny, ns, world, cam));
			futures_vector.push_back(std::async(std::launch::async, render_loop_test, nx, ny, y_start, y_end, ns, world, cam));
		}

		int current_threads_used = 0;
		for (auto &e : futures_vector) {
			// test ----------------
			// Use wait_for() with zero milliseconds to check thread status.
			auto status = e.wait_for(std::chrono::milliseconds(0));

			// Print status.
			if (status == std::future_status::ready) {
				std::cout << "Thread finished" << std::endl;
				current_threads_used--;
			}
			else {
				std::cout << "Thread still running" << std::endl;
				current_threads_used++;
			}
			std::cout << "current used threads: " << current_threads_used << std::endl;
			// test ----------------
			if (e.valid()) {
				myfile << e.get();
			}
		}

		myfile.close();
	}
	else std::cout << "Unable to open file";

	// time when finished
	auto end_timer = std::chrono::steady_clock::now();
	auto diff = end_timer - start_timer;
	std::cout << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	//--------------------------------

	return 0;
}
