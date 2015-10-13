#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <random>

#include "comp308.hpp"
#include "geometry.hpp"

using namespace std;
using namespace comp308;

vec3 random_vec3() {
	static default_random_engine re;
	uniform_real_distribution<float> dist(0, 1.0);
	return vec3(dist(re), dist(re), dist(re));
}

geometry::geometry() {
	m_color = normalize(random_vec3());
}

//geometry::geometry(vector<vector<vec3>> triangles) {
//	m_color = normalize(random_vec3());
//	allTriangles = triangles;
//}

geometry::geometry(string filename){
	m_color = normalize(random_vec3());

	readOBJ(filename);
	if (m_triangles.size() > 0) {
		//allTriangles = createDisplayListPoly();
		createDisplayListPoly();
	}
}

void geometry::readOBJ(string filename) {

	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0, 0, 0));
	m_uvs.push_back(vec2(0, 0));
	m_normals.push_back(vec3(0, 0, 1));


	ifstream objFile(filename);

	if (!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file " << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while (objFile.good()) {

		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);

		// Pull out mode from line
		string mode;
		objLine >> mode;

		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {

			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);

			}
			else if (mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);

			}
			else if (mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
				m_uvs.push_back(vt);

			}
			else if (mode == "f") {

				vector<vertex> verts;
				while (objLine.good()) {
					vertex v;

					//-------------------------------------------------------------
					// [Assignment 1] :
					// Modify the following to parse the bunny.obj. It has no uv
					// coordinates so each vertex for each face is in the format
					// v//vn instead of the usual v/vt/vn.
					//
					// Modify the following to parse the dragon.obj. It has no
					// normals or uv coordinates so the format for each vertex is
					// v instead of v/vt/vn or v//vn.
					//
					// Hint : Check if there is more than one uv or normal in
					// the uv or normal vector and then parse appropriately.
					//-------------------------------------------------------------

					// Assignment code (assumes you have all of v/vt/vn for each vertex)
					int i;
					objLine >> i;		// Scan in position index
					v.p = m_points[i];
					objLine.ignore(1);	// Ignore the '/' character
					objLine >> i;		// Scan in uv (texture coord) index
					v.t = m_uvs[i];
					objLine.ignore(1);	// Ignore the '/' character
					objLine >> i;		// Scan in normal index
					v.n = m_normals[i];

					verts.push_back(v);
				}

				// IFF we have 3 verticies, construct a triangle
				if (verts.size() == 3) {
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);

				}
			}
		}
	}

	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size() - 1 << " points" << endl;
	cout << m_uvs.size() - 1 << " uv coords" << endl;
	cout << m_normals.size() - 1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;


	//// If we didn't have any normals, create them
	//if (m_normals.size() <= 1) createNormals();
}

void geometry::createDisplayListPoly() {

	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);

	glBegin(GL_TRIANGLES);

	cout << m_triangles.size() << endl;

	for (triangle t : m_triangles) {

		glNormal3f(t.v[0].n.x, t.v[0].n.y, t.v[0].n.z);
		glTexCoord2f(t.v[0].t.x, t.v[0].t.y);
		glVertex3f(t.v[0].p.x, t.v[0].p.y, t.v[0].p.z);

		glNormal3f(t.v[1].n.x, t.v[1].n.y, t.v[1].n.z);
		glTexCoord2f(t.v[1].t.x, t.v[1].t.y);
		glVertex3f(t.v[1].p.x, t.v[1].p.y, t.v[1].p.z);

		glNormal3f(t.v[2].n.x, t.v[2].n.y, t.v[2].n.z);
		glTexCoord2f(t.v[2].t.x, t.v[2].t.y);
		glVertex3f(t.v[2].p.x, t.v[2].p.y, t.v[2].p.z);
	}
	glEnd();

	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}

void geometry::draw() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//Render geometry
	render();

	// Clean up
	glPopMatrix();

}

void geometry::render() {
	glShadeModel(GL_SMOOTH);
	glCallList(m_displayListPoly);
	//glColor3f(m_color.x, m_color.y, m_color.z);
	//glBegin(GL_TRIANGLES);
	//for (triangle t : allTriangles) {
	//	glNormal3f(t.v[0].n, t.v[1].n, t.v[2].n);
	//	//glTexCoord2f(t.v[0].t, t.v[1].t, t.v[2].t);
	//	glVertex3f(t.v[0].p, t.v[1].p, t.v[2].p);
	//}
	//glEnd();
	glBegin(GL_TRIANGLES);

	cout << m_triangles.size() << endl;

	for (triangle t : m_triangles) {

		glNormal3f(t.v[0].n.x, t.v[0].n.y, t.v[0].n.z);
		glTexCoord2f(t.v[0].t.x, t.v[0].t.y);
		glVertex3f(t.v[0].p.x, t.v[0].p.y, t.v[0].p.z);

		glNormal3f(t.v[1].n.x, t.v[1].n.y, t.v[1].n.z);
		glTexCoord2f(t.v[1].t.x, t.v[1].t.y);
		glVertex3f(t.v[1].p.x, t.v[1].p.y, t.v[1].p.z);

		glNormal3f(t.v[2].n.x, t.v[2].n.y, t.v[2].n.z);
		glTexCoord2f(t.v[2].t.x, t.v[2].t.y);
		glVertex3f(t.v[2].p.x, t.v[2].p.y, t.v[2].p.z);
	}
	glEnd();
}

vector<triangle> geometry::getTriangles() {
	return m_triangles;
}

void geometry::addToTriangles(triangle triangle) {
	m_triangles.push_back(triangle);
}

//vector<vec3> geometry::getNormals() {
//	return m_normals;
//}
//
//vector<vec2> geometry::getTextures() {
//	return m_uvs;
//}
//
//vector<vec3> geometry::getPoints() {
//	return m_points;
//}
//
//void geometry::setNormals(vector<vec3>) {
//
//}
//void geometry::setTextures(vector<vec2>) {
//
//}
//void geometry::setPoints(vector<vec3>) {
//
//}

geometry::~geometry(){}