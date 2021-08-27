#include <iostream>
#include <memory>
#include <vector>

#include <archive.h>
#include <archive_entry.h>
#include <fmt/core.h>
#include <rapidxml/rapidxml_utils.hpp>

#include "rapidxml_ext.h"

using namespace std;
using namespace rapidxml;

struct Point {
  float latitude;
  float longitude;
};

vector<Point> process_doc(xml_document<> *doc) {
  vector<Point> points;
  auto first_node = doc->first_node();
  if (strcmp(first_node->name(), "gpx") == 0) {
    for (auto *x_track = first_node->first_node(); x_track;
         x_track = x_track->next_sibling()) {
      for (auto *x_segm = x_track->first_node(); x_segm;
           x_segm = x_segm->next_sibling()) {
        for (auto *x_pt = x_segm->first_node(); x_pt;
             x_pt = x_pt->next_sibling()) {
          Point point;
          // printf("nn -> %s\n", x_pt->name());
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
          if (point.longitude > 0 && point.latitude > 0)
            points.emplace_back(point);
        }
      }
    }
  }
  return points;
}

vector<Point> parse_mem(char *buf, size_t size) {
  xml_document<> doc;
  // cout << buf;
  doc.parse<0>(buf);
  return process_doc(&doc);
}

vector<Point> parse_xml(char *filename) {
  file<> xmlFile(filename);
  xml_document<> doc;
  doc.parse<0>(xmlFile.data());
  return process_doc(&doc);
}

xml_document<> *build_kml() {
  xml_document<> *doc = new xml_document<>();
  xml_node<> *node = doc->allocate_node(node_element, "kml");
  xml_attribute<> *attr =
      doc->allocate_attribute("xmlns", "http://www.opengis.net/kml/2.2");
  node->append_attribute(attr);
  doc->append_node(node);
  auto don = doc->allocate_node(node_element, "Document");
  node->append_node(don);

  don->append_node(doc->allocate_node(node_element, "name", "Rides"));

  return doc;
}

xml_node<> *find_node(xml_node<> *doc, string name) {
  for (auto node = doc->first_node(); node; node = node->next_sibling()) {
    if (strcmp(node->name(), name.c_str())) {
      return node;
    } else {
      return find_node(node, name);
    }
  }
  return NULL;
}

void add_track(xml_document<> *doc, vector<Point> points, string num) {
  auto node = find_node(doc, "Document");
  if (node) {
    auto plm = doc->allocate_node(node_element, "Placemark");
    node->append_node(plm);

    plm->append_node(doc->allocate_node(node_element, "name", num.c_str()));

    auto line = doc->allocate_node(node_element, "LineString");
    plm->append_node(line);

    line->append_node(doc->allocate_node(node_element, "tesselate", "0"));
    line->append_node(
        doc->allocate_node(node_element, "altitudeMode", "clampToGround"));

    auto coords = doc->allocate_node(node_element, "coordinates");

    string buffer;
    buffer.append("\n");
    for (const auto &p : points) {
      auto ft = fmt::format("{},{},0 ", p.latitude, p.longitude);
      // cout << ft;
      buffer.append(ft);
    }
    buffer.append("\n");
    auto node_string = doc->allocate_string(buffer.c_str());
    coords->value(node_string);
    line->append_node(coords);
  }
}

vector<vector<Point>> read_archive(char *filename) {
  struct archive *a;
  struct archive_entry *entry;
  int r;

  vector<vector<Point>> all_points;

  a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  r = archive_read_open_filename(a, filename, 10240); // Note 1
  if (r != ARCHIVE_OK)
    exit(1);
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    auto fname = archive_entry_pathname(entry);
    if (strstr(fname, "activities/") && strcmp(fname, "activities/")) {
      // printf("%s\n", fname);
      size_t fsize = archive_entry_size(entry);
      auto buf_ptr = std::make_unique<uint8_t[]>(fsize+1); // add one byte for null char
      auto bytes = archive_read_data(a, buf_ptr.get(), fsize);
      if (bytes != fsize) {
        printf("Read non-full file, exiting!\n");
        exit(1);
      }
      printf("%s -> %zu\n", fname, fsize);
      buf_ptr.get()[fsize] = '\0'; // manually add null-char
      auto points = parse_mem((char *)buf_ptr.get(), bytes);
      printf("points: %li\n", points.size());
      all_points.emplace_back(points);
    }
    // archive_read_data_skip(a);  // Note 2
  }
  r = archive_read_free(a); // Note 3
  if (r != ARCHIVE_OK)
    exit(1);
  return all_points;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {

    char *filename = argv[1];
    auto all_points = read_archive(filename);
    for(const auto& points: all_points) {

    }
    // auto points = parse_xml(filename);
    // xml_document<> *final_doc = build_kml();
    // add_track(final_doc, points, fmt::format("{}", 0));
    // string buffer = "";
    // rapidxml::print(back_inserter(buffer), *final_doc, 0);
    // std::cout << buffer;
    // delete final_doc;
  }
  return 0;
}
