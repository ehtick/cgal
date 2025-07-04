#include <CGAL/SMDS_3/io_signature.h>
#include "Scene_c3t3_item.h"
#include <CGAL/SMDS_3/tet_soup_to_c3t3.h>
#include <CGAL/Three/CGAL_Lab_io_plugin_interface.h>
#include <CGAL/Three/Three.h>
#include <CGAL/IO/File_avizo.h>
#include <iostream>
#include <fstream>

#include <QMessageBox>

class CGAL_Lab_c3t3_binary_io_plugin :
  public QObject,
  public CGAL::Three::CGAL_Lab_io_plugin_interface
{
    Q_OBJECT
    Q_INTERFACES(CGAL::Three::CGAL_Lab_io_plugin_interface)
    Q_PLUGIN_METADATA(IID "com.geometryfactory.CGALLab.IOPluginInterface/1.90" FILE "c3t3_io_plugin.json")

public:
  QString name() const override { return "C3t3_io_plugin"; }
  QString nameFilters() const override { return "binary files (*.cgal);;ascii (*.mesh);;maya (*.ma);;avizo (*.tetra.am)"; }
  QString saveNameFilters() const override{
    return "binary files (*.cgal);;ascii (*.mesh);;maya (*.ma);;avizo (*.tetra.am);;OFF files (*.off)"; }
  QString loadNameFilters() const override {
    return "binary files (*.cgal);;ascii (*.mesh);;avizo (*.tetra.am)"; }
  bool canLoad(QFileInfo) const override;
  QList<Scene_item*> load(QFileInfo fileinfo, bool& ok, bool add_to_scene=true) override;

  bool canSave(const CGAL::Three::Scene_item*) override;
  bool save(QFileInfo fileinfo,QList<CGAL::Three::Scene_item*>& ) override;
  bool isDefaultLoader(const Scene_item* item) const override{
    if(qobject_cast<const Scene_c3t3_item*>(item))
      return true;
    return false;
  }

private:
  bool try_load_other_binary_format(std::istream& in, C3t3& c3t3);
  bool try_load_a_cdt_3(std::istream& in, C3t3& c3t3);
  void update_c3t3(C3t3& c3t3);
};


bool CGAL_Lab_c3t3_binary_io_plugin::canLoad(QFileInfo fi) const {
  if(!fi.suffix().contains("cgal"))
    return true;
  std::ifstream in(fi.filePath().toUtf8(),
                   std::ios_base::in|std::ios_base::binary);
  if(!in) {
    std::cerr << "Error! Cannot open file "
              << (const char*)fi.filePath().toUtf8() << std::endl;
    return false;
  }
  if (fi.suffix().toLower() == "cgal")
  {
    std::string line;
    std::istringstream iss;
    std::getline(in, line);
    iss.str(line);
    std::string keyword;
    if (iss >> keyword)
      if (keyword == "binary")
        return true;
  }
  else if (fi.suffix().toLower() == "am")
  {
    return true;
  }
  return false;
}

QList<Scene_item*>
CGAL_Lab_c3t3_binary_io_plugin::load(
    QFileInfo fileinfo, bool& ok, bool add_to_scene) {

    // Open file
    ok = true;
    std::ifstream in(fileinfo.filePath().toUtf8(),
                     std::ios_base::in|std::ios_base::binary);
    if(!in) {
      std::cerr << "Error! Cannot open file "
                << (const char*)fileinfo.filePath().toUtf8() << std::endl;
      ok = false;
      return QList<Scene_item*>();
    }
    Scene_c3t3_item* item = new Scene_c3t3_item();

    if(fileinfo.size() == 0)
    {
      CGAL::Three::Three::warning( tr("The file you are trying to load is empty."));
      item->setName(fileinfo.completeBaseName());
      if(add_to_scene)
        CGAL::Three::Three::scene()->addItem(item);
      return QList<Scene_item*>()<< item;
    }
    if(fileinfo.suffix().toLower() == "cgal")
    {
        item->setName(fileinfo.completeBaseName());

        if(item->load_binary(in)) {
          if(add_to_scene){
            item->resetCutPlane();
            CGAL::Three::Three::scene()->addItem(item);
          }
          return QList<Scene_item*>() << item;
        }

        item->c3t3().clear();
        in.seekg(0);
        if(try_load_other_binary_format(in, item->c3t3())) {
          item->resetCutPlane();
          item->c3t3_changed();
          item->changed();
          if(add_to_scene)
            CGAL::Three::Three::scene()->addItem(item);
          return QList<Scene_item*>()<< item;
        }

        item->c3t3().clear();
        in.seekg(0);
        if(try_load_a_cdt_3(in, item->c3t3())) {
          item->resetCutPlane();
          item->c3t3_changed();
          item->changed();
          if(add_to_scene)
            CGAL::Three::Three::scene()->addItem(item);
          return QList<Scene_item*>()<<item;
        }
    }
    else if (fileinfo.suffix().toLower() == "mesh")
    {
      in.close();
      in.open(fileinfo.filePath().toUtf8(), std::ios_base::in);//not binary
      CGAL_assertion(!(!in));

      item->setName(fileinfo.completeBaseName());
      item->set_valid(false);

      if(CGAL::SMDS_3::build_triangulation_from_file(in, item->c3t3().triangulation(),
         /*verbose = */false, /*replace_subdomain_0 = */false, /*allow_non_manifold = */true))
      {
        update_c3t3(item->c3t3());

        item->resetCutPlane();
        item->c3t3_changed();
        if(add_to_scene)
          CGAL::Three::Three::scene()->addItem(item);
        return QList<Scene_item*>()<<item;
      }
      else if(item->c3t3().triangulation().number_of_finite_cells() == 0)
      {
        QMessageBox::warning((QWidget*)NULL, tr("C3t3_io_plugin"),
                                       tr("No finite cell provided.\n"
                                          "Nothing to display."),
                                        QMessageBox::Ok);
      }
    }
    else if (fileinfo.suffix().toLower() == "am")
    {
      if (CGAL::IO::internal::is_avizo_tetra_format(in, "ascii"))
      {
        in.close();
        in.open(fileinfo.filePath().toUtf8(), std::ios_base::in);//not binary
        CGAL_assertion(!(!in));
      }

      item->setName(fileinfo.completeBaseName());

      if (CGAL::IO::read_AVIZO_TETRA(in, item->c3t3().triangulation()))
      {
        update_c3t3(item->c3t3());

        item->resetCutPlane();
        item->c3t3_changed();
        if (add_to_scene)
          CGAL::Three::Three::scene()->addItem(item);
        return QList<Scene_item*>() << item;
      }
    }

    // if all loading failed...
    delete item;
    ok = false;
    return QList<Scene_item*>();
}

bool CGAL_Lab_c3t3_binary_io_plugin::canSave(const CGAL::Three::Scene_item* item)
{
  // This plugin supports c3t3 items.
  return qobject_cast<const Scene_c3t3_item*>(item);
}

bool
CGAL_Lab_c3t3_binary_io_plugin::
save(QFileInfo fileinfo, QList<Scene_item *> &items)
{
  Scene_item* item = items.front();
  const Scene_c3t3_item* c3t3_item = qobject_cast<const Scene_c3t3_item*>(item);
  if(nullptr == c3t3_item) {
    return false;
  }

  QString path = fileinfo.absoluteFilePath();

  if(path.endsWith(".cgal"))
  {
    if(!c3t3_item->is_valid())
    {
      QMessageBox::warning(CGAL::Three::Three::mainWindow(), "",
                           "The c3t3_item is not valid. You cannot save it in binary cgal format.");
      return false;
    }
    std::ofstream out(fileinfo.filePath().toUtf8(),
                      std::ios_base::out|std::ios_base::binary);
    bool ok = out && c3t3_item->save_binary(out);
    if(!ok)
      return false;
    else
    {
      items.pop_front();
      return true;
    }
  }

  else  if (fileinfo.suffix() == "mesh")
  {
    std::ofstream medit_file (qPrintable(path));
    CGAL::IO::write_MEDIT(medit_file, c3t3_item->c3t3(),
                          CGAL::parameters::rebind_labels(true)
                          .show_patches(true));
    items.pop_front();
    return true;
  }
  else if (fileinfo.suffix() == "ma")
  {
    std::ofstream maya_file (qPrintable(path));
    c3t3_item->c3t3().output_to_maya(
          maya_file,true);
    items.pop_front();
    return true;
  }
  else if (fileinfo.suffix() == "am")
  {
    std::ofstream avizo_file (qPrintable(path));
    CGAL::IO::output_to_avizo(avizo_file, c3t3_item->c3t3());
    items.pop_front();
    return true;
  }
  else if (fileinfo.suffix() == "off")
  {
    std::ofstream off_file(qPrintable(path));
    c3t3_item->c3t3().output_facets_in_complex_to_off(off_file);
    items.pop_front();
    return true;
  }
  else
    return false;
}

struct Fake_mesh_domain {
  typedef CGAL::Tag_true Has_features;
  typedef int Subdomain_index;
  typedef std::pair<int,int> Surface_patch_index;
  typedef int Curve_index;
  typedef int Corner_index;
  typedef std::variant<Subdomain_index,Surface_patch_index> Index;
};

typedef Geom_traits Fake_gt;
typedef CGAL::Mesh_vertex_base_3<Fake_gt, Fake_mesh_domain> Fake_vertex_base;
typedef CGAL::Compact_mesh_cell_base_3<Fake_gt, Fake_mesh_domain> Fake_cell_base;
typedef CGAL::Triangulation_data_structure_3<Fake_vertex_base,Fake_cell_base> Fake_tds;
typedef CGAL::Regular_triangulation_3<Fake_gt, Fake_tds> Fake_tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<
  Fake_tr,
  Fake_mesh_domain::Corner_index,
  Fake_mesh_domain::Curve_index> Fake_c3t3;

template <class Cb = CGAL::Triangulation_cell_base_3<EPICK> >
struct Fake_CDT_3_cell_base : public Cb
{
  typedef Cb Base;
  int face_id[4];

  template < typename TDS2 >
  struct Rebind_TDS {
    typedef typename Base::template Rebind_TDS<TDS2>::Other   Cb2;
    typedef Fake_CDT_3_cell_base<Cb2>  Other;
  };

  static std::string io_signature() {
    return CGAL::Get_io_signature<Base>()() + "+(" +
           CGAL::Get_io_signature<int>()() + ")[4]";
  }
  friend
  std::istream&
  operator>>( std::istream& is, Fake_CDT_3_cell_base& c) {
    is >> static_cast<Cb&>(c);
    for( int li = 0; li < 4; ++li ) {
      if( CGAL::IO::is_ascii(is) )
        is >> c.face_id[li];
      else
        CGAL::read( is, c.face_id[li] );
    }
    return is;
  }
};


typedef CGAL::Triangulation_data_structure_3<CGAL::Triangulation_vertex_base_3<EPICK>,
                                             Fake_CDT_3_cell_base<> > Fake_CDT_3_TDS;
typedef CGAL::Triangulation_3<EPICK, Fake_CDT_3_TDS> Fake_CDT_3;

typedef Fake_mesh_domain::Surface_patch_index Fake_patch_id;

template <typename Tr1, typename Tr2>
struct Update_vertex
{
  typedef Fake_mesh_domain::Surface_patch_index Sp_index;
  typedef typename Tr1::Vertex                  V1;
  typedef typename Tr2::Vertex                  V2;
  typedef typename Tr2::Point                   Point;

  V2 operator()(const V1&)
  {
    return V2();
  }

  bool operator()(const V1& v1, V2& v2)
  {
    v2.set_point(Point(v1.point()));
    v2.set_dimension(v1.in_dimension());
    v2.set_special(v1.is_special());
    switch(v1.in_dimension()) {
    case 2:
    {
      const typename V1::Index& index = v1.index();
      const Sp_index sp_index = std::get<Sp_index>(index);
      v2.set_index((std::max)(sp_index.first, sp_index.second));
    }
      break;
    default:// -1, 0, 1, 3
      v2.set_index(std::get<int>(v1.index()));
    }
    return true;
  }
}; // end struct Update_vertex

struct Update_cell {

  typedef Fake_mesh_domain::Surface_patch_index Sp_index;

  template <typename C1, typename C2>
  bool operator()(const C1& c1, C2& c2) {
    c2.set_subdomain_index(c1.subdomain_index());
    for(int i = 0; i < 4; ++i) {
      const Sp_index sp_index = c1.surface_patch_index(i);
      c2.set_surface_patch_index(i, (std::max)(sp_index.first,
                                               sp_index.second));
      CGAL_assertion(c1.is_facet_on_surface(i) ==
                     c2.is_facet_on_surface(i));
    }
    return true;
  }
}; // end struct Update_cell


template <typename Tr1, typename Tr2>
struct Update_vertex_from_CDT_3 {
  // Tr1 and Tr2's point types might be different

  typedef typename Tr1::Vertex          V1;
  typedef typename Tr2::Vertex          V2;
  typedef typename Tr2::Point           Point;

   V2 operator()(const V1&)
  {
    return V2();
  }
  void operator()(const V1& v1, V2& v2)
  {
    v2.set_point(Point(v1.point()));
    v2.set_dimension(2);
    v2.set_special(false);
  }
}; // end struct Update_vertex

struct Update_cell_from_CDT_3 {

  typedef Fake_mesh_domain::Surface_patch_index Sp_index;

  template <typename C1,typename C2>
  void operator()(const C1& c1, C2& c2) {
    c2.set_subdomain_index(1);
    for(int i = 0; i < 4; ++i) {
      c2.set_surface_patch_index(i, c1.face_id[i]+1);
    }
  }
}; // end struct Update_cell

bool
CGAL_Lab_c3t3_binary_io_plugin::
try_load_a_cdt_3(std::istream& is, C3t3& c3t3)
{
  std::cerr << "Try load a CDT_3...";
  std::string s;
  if(!(is >> s)) return false;
  bool binary = (s == "binary");
  if(binary) {
    if(!(is >> s)) return false;
  }
  if (s != "CGAL" ||
      !(is >> s) ||
      s != "c3t3")
  {
    return false;
  }
  std::getline(is, s);
  if(s != "") {
    if(s != std::string(" ") + CGAL::Get_io_signature<Fake_CDT_3>()()) {
      std::cerr << "load_binary_file:"
                << "\n  expected format: " << CGAL::Get_io_signature<Fake_CDT_3>()()
                << "\n       got format:" << s << std::endl;
      return false;
    }
  }
  if(binary) CGAL::IO::set_binary_mode(is);
  if(c3t3.triangulation().file_input<
       Fake_CDT_3,
       Update_vertex_from_CDT_3<Fake_CDT_3, C3t3::Triangulation>,
       Update_cell_from_CDT_3>(is))
  {
    c3t3.rescan_after_load_of_triangulation();
    std::cerr << "Try load a CDT_3... DONE";
    return true;
  }
  else {
    return false;
  }
}

void
CGAL_Lab_c3t3_binary_io_plugin::
update_c3t3(C3t3& c3t3)
{
  using Cell_handle = C3t3::Triangulation::Cell_handle;

  c3t3.rescan_after_load_of_triangulation(); //fix counters for facets and cells
  for (Cell_handle cit : c3t3.triangulation().finite_cell_handles())
  {
    CGAL_assertion(cit->subdomain_index() >= 0);
    if (cit->subdomain_index() != C3t3::Triangulation::Cell::Subdomain_index())
      c3t3.add_to_complex(cit, cit->subdomain_index());

    for (int i = 0; i < 4; ++i)
    {
      if (cit->surface_patch_index(i) > 0)
        c3t3.add_to_complex(cit, i, cit->surface_patch_index(i));
    }
  }

  //if there is no facet in the complex, we add the border facets.
  if (c3t3.number_of_facets_in_complex() == 0)
  {
    for (C3t3::Facet fit : c3t3.triangulation().finite_facets())
    {
      Cell_handle c = fit.first;
      Cell_handle nc = c->neighbor(fit.second);

      // By definition, Subdomain_index() is supposed to be the id of the exterior
      if (c->subdomain_index() != C3t3::Triangulation::Cell::Subdomain_index() &&
          nc->subdomain_index() == C3t3::Triangulation::Cell::Subdomain_index())
      {
        // Color the border facet with the index of its cell
        c3t3.add_to_complex(c, fit.second, c->subdomain_index());
      }
    }
  }

}

//Generates a compilation error.
bool
CGAL_Lab_c3t3_binary_io_plugin::
try_load_other_binary_format(std::istream& is, C3t3& c3t3)
{
  CGAL::IO::set_ascii_mode(is);
  std::string s;
  if(!(is >> s)) return false;
  bool binary = false;
  if(s == "binary") {
    binary = true;
    if(!(is >> s)) return false;
  }
  if( s != "CGAL" ||
      !(is >> s) ||
      s != "c3t3")
  {
    return false;
  }
  std::getline(is, s);
  if(s != "") {
    if(s != std::string(" ") + CGAL::Get_io_signature<Fake_c3t3>()()) {
      std::cerr << "CGAL_Lab_c3t3_binary_io_plugin::try_load_other_binary_format:"
                << "\n  expected format: " << CGAL::Get_io_signature<Fake_c3t3>()()
                << "\n       got format:" << s << std::endl;
      return false;
    }
  }
  if(binary) CGAL::IO::set_binary_mode(is);
  else CGAL::IO::set_ascii_mode(is);
  std::istream& f_is = c3t3.triangulation().file_input<
                         Fake_c3t3::Triangulation,
                         Update_vertex<Fake_c3t3::Triangulation, C3t3::Triangulation>,
                         Update_cell>(is);

  c3t3.rescan_after_load_of_triangulation();
  return f_is.good();
}

#include <QtPlugin>
#include "C3t3_io_plugin.moc"
