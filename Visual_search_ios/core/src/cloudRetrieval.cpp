#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/console/parse.h>
#include <pcl/search/impl/flann_search.hpp>
#include <typedefs.hpp>
#include "configuration/configuration.hpp"
#include "recognition_database/recognition_database.h"


int k = 10;
float leaf_resolution = 0.01f;
bool enable_resolution = true;
bool enable_scaling = false;
bool compute_on_full = true;
std::string descriptor_type = "esf";
std::string output_path_result = "../results.json";

void getCategories(std::string path_to_data, std::vector<std::string>& categories){
    
    for(boost::filesystem::directory_iterator it(path_to_data); it!=boost::filesystem::directory_iterator(); ++it)
    {
         std::string path_current = it->path().c_str();
        boost::filesystem::path p(path_current);
        std::string filename = p.filename().c_str();
        if (boost::filesystem::is_directory(path_current)){
            categories.push_back(filename);
        }
    }

}

void
showHelp (char *filename)
{
    std::cout << std::endl;
    std::cout << "***************************************************************************" << std::endl;
    std::cout << "*                                                                         *" << std::endl;
    std::cout << "*                 Cloud Retrieval using similarity search                 *" << std::endl;
    std::cout << "*                                                                         *" << std::endl;
    std::cout << "***************************************************************************" << std::endl << std::endl;
    std::cout << "Usage: " << filename << "config_file -query object.pcd -trained tained_dataset [Options]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "     -h:                     Show this help." << std::endl;
    std::cout << "     -descriptor:            change descriptor. Available : esf, vfh, cvfh, ourcvfh, gshot. Per default : esf " << std::endl;
    std::cout << "     -output:                where to save the json results file. Per default : ../results.json" << std::endl;
    std::cout << "     -leaf_resolution:       For cloud resolution invariance,. Per defaut : 0.01 \n" << std::endl;
    std::cout << "     -compute_full:          Use full objects or views,. Per defaut : true \n" << std::endl;
    std::cout << "     -k:               number of results to find \n" << std::endl;
    
}

struct path_leaf_string
{
    std::string operator()(const boost::filesystem::directory_entry& entry) const
    {
        return entry.path().leaf().string();
    }
};
void read_directory(const std::string& name, std::vector<std::string>& v)
{
    boost::filesystem::path p(name);
    boost::filesystem::directory_iterator start(p);
    boost::filesystem::directory_iterator end;
    std::transform(start, end, std::back_inserter(v), path_leaf_string());
}

int main(int argc, char** argv)
{
    
    //showHelp (argv[0]);
    
    //Show help
    if (pcl::console::find_switch (argc, argv, "-h"))
    {
        showHelp (argv[0]);
        exit (0);
    }
    //If not enough parameters
    if (argc < 4)
    {
        std::cerr << "Not enough aguments " << std::endl;
        showHelp (argv[0]);
        return (-1);
    }
    

    std::string configuration_file = argv[1]; // Configuration file to take in account the parameters
    Config cfg;
    //Get parameters from the configuration file
    if (getConfiguration (configuration_file, cfg) < 0)
        return (-1);
    
    bool resolution_activated = cfg.similaritySearch.enable_resolution;
    bool scale_activated = enable_scaling;
   
    std::string query;
    if(pcl::console::parse_argument(argc, argv, "-query", query) == -1)
    {
        std::cerr<<"Please specify the query (.pcd file)"<<std::endl;
        return -1;
    }
    std::string database_trained;
    if(pcl::console::parse_argument(argc, argv, "-trained", database_trained) == -1)
    {
        std::cerr<<"Please specify the trained database "<<std::endl;
        return -1;
    }
    
    std::vector<std::string> categories;
    getCategories(database_trained,categories);

    for (int i = 0;i< categories.size();i++){
        std::cout << "[Categories avaialble] : " << categories.at(i) << std::endl;
    }

    
    pcl::console::parse_argument (argc, argv, "-descriptor", descriptor_type);
    pcl::console::parse_argument (argc, argv, "-output", output_path_result);
    pcl::console::parse_argument (argc, argv, "-k", k);
    pcl::console::parse_argument (argc, argv, "-leaf_resolution", leaf_resolution);
    std::string activate ="";
    pcl::console::parse_argument (argc, argv, "-compute_full", activate);
    
    if (activate == "true"){
        compute_on_full = true;
    }else{
        compute_on_full = false;
    }
    
    std::cout << compute_on_full << std::endl;

    
    if (!boost::filesystem::exists (query)){
        std::cerr<<"[ERROR] query empty - Please specify the query again using -query "<<std::endl;
        return -1;
    }
    //For quick testing
    //Different distances:
    //chisquare distance flann::ChiSquareDistance<float>
    //L2 euclidean distance flann::L2<float>
    //Manhattan distance flann::L1<float>
    //Minkowski distance flann::MinkowskiDistance<float>
    //MaxDistance flann::MaxDistance<float>
    //HistIntersectionDistance flann::HistIntersectionDistance<float>
    //HistIntersectionUnionDistance flann::HistIntersectionUnionDistance<float>
  /*  enum Distance {chisquare, L2, L1, hellinger, histIntersection, histIntersectionUnion};//HistIntersectionUnion not working with kdtree
    Distance d = histIntersectionUnion;
    switch(d)
    {
        case chisquare  :
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::ChiSquareDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::ChiSquareDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
        case L2:
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::L2<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::L2<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
        case L1 :
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::L1<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::L1<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
        case hellinger:
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::HellingerDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::HellingerDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
        case histIntersection :
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::HistIntersectionDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::HistIntersectionDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
        case histIntersectionUnion :
            if (descriptor_type.compare("vfh") == 0){
                RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::HistIntersectionUnionDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }else {
                RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::HistIntersectionUnionDistance<float> > db;
                db.setDescriptor(descriptor_type);
                db.globalMatchingUsingViewOfCloud(query,database_trained,k);
                db.saveResultsDetectionsJSON(output_path_result);
            }
            break;
    }*/
    
    std::cout << "[INFO] Trained dataset path : " <<database_trained << std::endl;
    
    if (descriptor_type.compare("vfh") == 0){
        
        RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::L1<float> > db;
        db.setDescriptor(descriptor_type);
        db.setComputeOnFull(compute_on_full);
        if (resolution_activated){
            db.setResolutionMode(true);
            db.setResolution(cfg.filtering.leaf_resolution);
        }
        if (scale_activated){
            db.setScale(scale_activated);
        }
        db.globalMatchingUsingViewOfCloud(query,database_trained, categories,k);
        db.saveResultsDetectionsJSON(output_path_result);
        
    }else if (descriptor_type.compare("cvfh") == 0){
        
        RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::ChiSquareDistance<float> > db;
        db.setDescriptor(descriptor_type);
        db.setComputeOnFull(compute_on_full);
        if (resolution_activated){
            db.setResolutionMode(true);
            db.setResolution(cfg.filtering.leaf_resolution);
        }
        if (scale_activated){
            db.setScale(scale_activated);
        }
        db.globalMatchingUsingViewOfCloud(query,database_trained,categories,k);
        db.saveResultsDetectionsJSON(output_path_result);
        
        
    }else if (descriptor_type.compare("ourcvfh") == 0){
        
        RecognitionDatabase<pcl::PointXYZ, pcl::VFHSignature308, flann::ChiSquareDistance<float> > db;
        db.setDescriptor(descriptor_type);
        db.setComputeOnFull(compute_on_full);
        if (resolution_activated){
            db.setResolutionMode(true);
            db.setResolution(cfg.filtering.leaf_resolution);
        }
        if (scale_activated){
            db.setScale(scale_activated);
        }
        db.globalMatchingUsingViewOfCloud(query,database_trained,categories,k);
        db.saveResultsDetectionsJSON(output_path_result);
        
        
    }else if (descriptor_type.compare("esf") == 0){
        
        RecognitionDatabase<pcl::PointXYZ, pcl::ESFSignature640, flann::L2<float> > db;
        db.setDescriptor(descriptor_type);
        db.setComputeOnFull(compute_on_full);
        if (resolution_activated){
            db.setResolutionMode(true);
            db.setResolution(cfg.filtering.leaf_resolution);
        }
        if (scale_activated){
            db.setScale(scale_activated);
        }
        db.globalMatchingUsingViewOfCloud(query,database_trained,categories,k);
        db.saveResultsDetectionsJSON(output_path_result);
        
        
    }
    else if (descriptor_type.compare("gshot") == 0 || descriptor_type.compare("gshotPyramid") == 0 ){
        
        RecognitionDatabase<pcl::PointXYZ, pcl::SHOT352, flann::L2<float> > db;
        db.setDescriptor(descriptor_type);
        db.setComputeOnFull(compute_on_full);
        if (resolution_activated){
            db.setResolutionMode(true);
            db.setResolution(leaf_resolution);
        }
        if (scale_activated){
            db.setScale(scale_activated);
        }
        db.globalMatchingUsingViewOfCloud(query,database_trained,categories,k);
        db.saveResultsDetectionsJSON(output_path_result);
        
        
    }
    else {
        std::cerr<<"Unknwon descriptor type"<<std::endl;
        return -1;
    }
    
    

    
    return 0;
}
