#include <iostream>
#include <vector>

#include "rapidxml_utils.hpp"
#include "rapidxml_ext.h"

using namespace std;
using namespace rapidxml;

struct Point {
  float latitude;
  float longitude;
};

vector<Point> parse_xml(char *filename) {
  file<> xmlFile(filename);
  xml_document<> doc;

  vector<Point> points;

  doc.parse<0>(xmlFile.data());
  auto first_node = doc.first_node();
  if (strcmp(first_node->name(), "gpx") == 0) {
    for (auto *x_track = first_node->first_node(); x_track;
         x_track = x_track->next_sibling()) {
      for (auto *x_segm = x_track->first_node(); x_segm;
           x_segm = x_segm->next_sibling()) {
        for (auto *x_pt = x_segm->first_node(); x_pt;
             x_pt = x_pt->next_sibling()) {
          Point point;
          for (auto *x_pt_attr = x_pt->first_attribute(); x_pt_attr;
               x_pt_attr = x_pt_attr->next_attribute()) {
            if (strcmp(x_pt_attr->name(), "lat")) {
              point.latitude = atof(x_pt_attr->value());
            } else if (strcmp(x_pt_attr->name(), "lon")) {
              point.longitude = atof(x_pt_attr->value());
            }
          }
          // printf("pt.lat -> %f\n", point.latitude);
          // printf("pt.lon -> %f\n", point.longitude);
          points.emplace_back(point);
        }
      }
    }
  }
  return points;
}

xml_document<>* build_kml(vector<Point>) {
  xml_document<> *doc = new xml_document<>();
  xml_node<> *node = doc->allocate_node(node_element, "kml");
  xml_attribute<> *attr = doc->allocate_attribute("xmlns", "http://www.opengis.net/kml/2.2");
  node->append_attribute(attr);
  doc->append_node(node);
  auto don = doc->allocate_node(node_element, "Document");
  node->append_node(don);

  auto name = doc->allocate_node(node_element, "name", "Ride");

  auto plm = doc->allocate_node(node_element, "Placemark");
  don->append_node(plm);

  auto line = doc->allocate_node(node_element, "LineString");
  plm->append_node(line);

  line->append_node(doc->allocate_node(node_element, "tesselate", "0"));
  line->append_node(doc->allocate_node(node_element, "altitudeMode", "clampToGround"));

  return doc;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    char *filename = argv[1];
    auto points = parse_xml(filename);
    xml_document<> *final_doc = build_kml(points);
    string buffer;
    rapidxml::print(back_inserter(buffer), *final_doc, 0);
    std::cout << buffer;
    delete final_doc;
  }
  return 0;
}
