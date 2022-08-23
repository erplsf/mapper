#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include <archive.h>
#include <archive_entry.h>

#include <rapidxml/rapidxml_utils.hpp>

#include "rapidxml_ext.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>

using namespace std;
using namespace rapidxml;

struct Point {
  float latitude;
  float longitude;
};

vector<Point> process_doc(xml_document<> *const doc) {
  vector<Point> points;
  auto first_node = doc->first_node();
  if (strcmp(first_node->name(), "gpx") == 0) {
    for (auto *x_track = first_node->first_node(); x_track;
         x_track = x_track->next_sibling()) {
      for (auto *x_segm = x_track->first_node(); x_segm;
           x_segm = x_segm->next_sibling()) {
        for (auto *x_pt = x_segm->first_node(); x_pt;
             x_pt = x_pt->next_sibling()) {
          optional<float> lat, lon;
          // printf("nn -> %s\n", x_pt->name());
          for (auto *x_pt_attr = x_pt->first_attribute(); x_pt_attr;
               x_pt_attr = x_pt_attr->next_attribute()) {
            if (strcmp(x_pt_attr->name(), "lat") == 0) {
              // printf("lat %s\n", x_pt_attr->value());
              lat = atof(x_pt_attr->value());
            } else if (strcmp(x_pt_attr->name(), "lon") == 0) {
              // printf("lon %s\n", x_pt_attr->value());
              lon = atof(x_pt_attr->value());
            }
          }
          // printf("pt.lat -> %f\n", point.latitude);
          // printf("pt.lon -> %f\n", point.longitude);
          if (lat && lon) {
            Point point{};
            point.latitude = *lat;
            point.longitude = *lon;
            // if (point.longitude < 1 || point.latitude < 1) {
            //   auto ft = fmt::format("{},{},0 ", point.latitude,
            //   point.longitude); printf("ipd: %s\n", ft.c_str()); exit(1);
            // }
            points.emplace_back(point);
          }
        }
      }
    }
  }
  return points;
}

vector<Point> parse_mem(char *const buf) {
  xml_document<> doc;
  // cout << buf;
  doc.parse<0>(buf);
  return process_doc(&doc);
}

vector<Point> parse_xml(char const *const filename) {
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
  auto fol = doc->allocate_node(node_element, "Folder");
  fol->append_node(doc->allocate_node(node_element, "name", "Rides"));
  don->append_node(fol);

  auto styl = doc->allocate_node(node_element, "Style");
  styl->append_attribute(doc->allocate_attribute("id", "line-style"));
  don->append_node(styl);

  auto ls = doc->allocate_node(node_element, "LineStyle");
  styl->append_node(ls);
  ls->append_node(
      doc->allocate_node(node_element, "color", "ff00ffff")); // aabbggrr
  // ls->append_node(doc->allocate_node(node_element, "colorMode", "normal"));
  // ls->append_node(doc->allocate_node(node_element, "scale", "1"));
  ls->append_node(doc->allocate_node(node_element, "width", "1.5"));

  return doc;
}

xml_node<> *find_node(xml_node<> const *const doc, string const name) {
  for (auto node = doc->first_node(); node; node = node->next_sibling()) {
    if (strcmp(node->name(), name.c_str()) == 0) {
      return node;
    } else {
      auto fn = find_node(node, name);
      if (fn)
        return fn;
    }
  }
  return nullptr;
}

void add_track(xml_document<> *const doc, vector<Point> const points,
               string_view const num) {
  auto node = find_node(doc, "Folder");
  if (node) {
    // printf("ia: %s\n", node->name());
    // exit(1);
    auto plm = doc->allocate_node(node_element, "Placemark");
    node->append_node(plm);
    plm->append_node(
        doc->allocate_node(node_element, "styleUrl", "#line-style"));

    auto name = doc->allocate_string(num.data());
    // string internal = num;
    plm->append_node(doc->allocate_node(node_element, "name", name));

    auto line = doc->allocate_node(node_element, "LineString");
    plm->append_node(line);

    line->append_node(doc->allocate_node(node_element, "tesselate", "0"));
    line->append_node(
        doc->allocate_node(node_element, "altitudeMode", "clampToGround"));

    auto coords = doc->allocate_node(node_element, "coordinates");

    string buffer;
    buffer.append("\n");
    for (const auto &point : points) {
      auto ft = fmt::format("{},{},0 ", point.longitude, point.latitude);
      // cout << ft;
      buffer.append(ft);
    }
    buffer.append("\n");
    auto node_string = doc->allocate_string(buffer.c_str());
    coords->value(node_string);
    line->append_node(coords);
  }
}

vector<vector<Point>> read_archive(const char *filename) {
  archive *a = archive_read_new();
  archive_entry *entry = nullptr;

  vector<vector<Point>> all_points;

  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  constexpr int BLOCK_SIZE = 10240;
  int r = archive_read_open_filename(a, filename, BLOCK_SIZE); // Note 1
  if (r != ARCHIVE_OK) {
    // TODO: add error extraction
    exit(1);
  }
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    auto fname = archive_entry_pathname(entry);
    if (strstr(fname, "activities/") != nullptr &&
        strcmp(fname, "activities/") != 0) {
      // printf("%s\n", fname);
      auto fsize = archive_entry_size(entry);
      auto buffer = std::make_unique<uint8_t[]>(
          fsize + 1); // NOLINT: We want array of dynamic size plus one byte for
                      // null terminator
      auto bytes = archive_read_data(a, buffer.get(), fsize);
      assert(bytes == fsize);
      if (bytes != fsize) {
        // printf("Read non-full file, exiting!\n");
        // TODO: add error
        exit(1);
      }
      // printf("%s -> %zu\n", fname, fsize);
      buffer.get()[fsize] = '\0'; // manually add null-char
      auto points = parse_mem(reinterpret_cast<char *>(
          buffer.get())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      // printf("points: %li\n", points.size());
      all_points.emplace_back(points);
    }
    // archive_read_data_skip(a);  // Note 2
  }
  r = archive_read_free(a); // Note 3
  if (r != ARCHIVE_OK)
    // TODO: add error extraction
    exit(1);
  return all_points;
}

int main(int const argc, char const *const argv[]) {
  if (argc == 2) {
    string_view filename = argv[1];
    xml_document<> *final_doc = build_kml();
    auto all_points = read_archive(filename.data());
    int i = 0;
    for (const auto &points : all_points) {
      i++;
      auto name = fmt::format("Ride {}", i);
      add_track(final_doc, points, name);
      // if (i == halfsize) break;
    }
    string buffer = "";
    rapidxml::print(back_inserter(buffer), *final_doc, 0);
    std::cout << buffer;
    delete final_doc;
  }
  return 0;
}
