#include <iostream>
#include <vector>

#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

class Point {
  float latitude;
  float longitude;
  float elevation; // optional

public:
  Point(float lat, float lon, float ele = 0)
      : latitude(lat), longitude(lon), elevation(ele){};
};

class Segment {
  vector<Point> points;

public:
  void add_point(Point p) {
    points.push_back(p);
  }
};

class Track {
  vector<Segment> segments;
};

Track parse_xml(char *filename) {
  Track track;
  file<> xmlFile(filename);
  xml_document<> doc;

  doc.parse<0>(xmlFile.data());
  auto first_node = doc.first_node();
  if (strcmp(first_node->name(), "gpx") == 0) {
    for (auto *x_track = first_node->first_node(); x_track;
         x_track = x_track->next_sibling()) {
      for (auto *x_segm = x_track->first_node(); x_segm;
           x_segm = x_segm->next_sibling()) {
        Segment segment;
        for (auto *x_pt = x_segm->first_node(); x_pt;
             x_pt = x_pt->next_sibling()) {
          // printf("p -> %s", x_pt->value());
          Point point(1, 2, 3);
          segment.add_point(point);
        }
      }
    }
  }
  return track;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    // printf("%s", argv[1]);
    char *filename = argv[1];
    Track track = parse_xml(argv[1]);
  }
  return 0;
}
