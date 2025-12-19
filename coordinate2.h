#ifndef COORDINATE2_H
#define COORDINATE2_H

#include <cmath>
#include <iostream>
#include <vector>
#include <stdexcept>

const double pi = 3.14159265358979;

/* Basic 2D coordinate class */
class Coordinate2
{
public:
    Coordinate2() : x{0},y{0},_prec{1e-10} { }
    Coordinate2(double x_in,double y_in) :x{x_in},y{y_in},_prec{1e-10} { }
    Coordinate2(const Coordinate2& c) = default; // Default memberwise copy constructor
    Coordinate2& operator=(const Coordinate2& c) = default; // Default memberwise assignment

    double x,y; // x and y are public

    Coordinate2& operator+=(const Coordinate2& rhs) {x+=rhs.x,y+=rhs.y; return *this;}
    Coordinate2& operator+=(double f) {x+=f,y+=f; return *this;}
    Coordinate2& operator-=(const Coordinate2& rhs) {x-=rhs.x,y-=rhs.y; return *this;}
    Coordinate2& operator-=(double f) {x-=f,y-=f; return *this;}
    Coordinate2& operator*=(double f) {x*=f,y*=f; return *this;}
    Coordinate2& operator/=(double f) {x/=f,y/=f; return *this;}

    double prec() const {return _prec;}
    void prec(double pin) {
        if(pin>0.0) _prec = pin;
        else throw std::invalid_argument("Coordinate precision cannot be negative or zero.");
    }

    // Utility functions
    double magnitude() const {return std::sqrt(std::pow(x,2)+std::pow(y,2));}
    double angle(bool degrees=false) const {
        if(magnitude() < _prec) throw std::logic_error("Cannot calculate angle of zero vector.");
        if(degrees) return std::atan2(y,x)*180/pi;
        else return std::atan2(y,x);
    }
    double distance(const Coordinate2& c) const {return std::sqrt(std::pow(x-c.x,2)+std::pow(y-c.y,2));}
    double angle(const Coordinate2& c, bool degrees=false) const {
        if((magnitude() < _prec)||(c.magnitude() < c.prec())) throw std::logic_error("Cannot calculate angle when one vector is the zero vector.");
        if(degrees) return std::atan2(y-c.y,x-c.x)*180/pi;
        else return std::atan2(y-c.y,x-c.x);
    }

    void print(bool newline = true) const {
        if(newline) std::cout << "("<<x<<", "<<y<<")\n";
        else std::cout << "("<<x<<", "<<y<<")";
    }
private:
    double _prec;
};

// Coordinate arithmetic operations
inline Coordinate2 operator+(Coordinate2 lhs, const Coordinate2& rhs) {return lhs += rhs;}
inline Coordinate2 operator-(Coordinate2 lhs, const Coordinate2& rhs) {return lhs -= rhs;}
inline Coordinate2 operator-(Coordinate2 lhs) {return lhs *= -1.0;}

// Scalar arithmetic operations
inline Coordinate2 operator+(Coordinate2 lhs, double f) {return lhs += f;}
inline Coordinate2 operator-(Coordinate2 lhs, double f) {return lhs -= f;}
inline Coordinate2 operator*(Coordinate2 lhs, double f) {return lhs *= f;}
inline Coordinate2 operator/(Coordinate2 lhs, double f) {return lhs /= f;}

// Comparison
inline bool operator==(const Coordinate2& lhs, const Coordinate2& rhs)
{
    using std::abs;
    double cprec = lhs.prec();
    if(rhs.prec() > cprec) cprec = rhs.prec();
    return (abs(lhs.x - rhs.x) < cprec) && (abs(lhs.y - rhs.y) < cprec);
}


inline double rad2deg(double angle_rad)
{
    return angle_rad*180/pi;
}
inline double deg2rad(double angle_deg)
{
    return angle_deg*pi/180;
}
inline double distance_from_line(const Coordinate2& P,
                                 const Coordinate2& LineStart,const Coordinate2& LineEnd)
{
    // Calculate the distance from a point to a line
    // First calculate point projection onto the line
    // If it projects onto the line, then calculate orthogonal distance
    // Otherwise calculate distance to endpoints
    using std::pow;
    using std::abs;
    using std::sqrt;

    double dx,dy;
    dx = LineEnd.x-LineStart.x;
    dy = LineEnd.y-LineStart.y;
    Coordinate2 delta(dx,dy);

    double ip = (P.x-LineStart.x)*delta.x+(P.y-LineStart.y)*delta.y;
    bool proj = (0 <= ip && ip <= pow(delta.magnitude(),2));
    // Thanks Daniel Fischer on SE

    double d;

    if(proj)
    {
        // Orthogonal distance
        // (delta.y*P.x-delta.x*P.y+x2*y1-y2*x1)/std::sqrt(delta.y*delta.y+delta.x*delta.x);
        d = abs(delta.y*P.x-delta.x*P.y+LineEnd.x*LineStart.y-LineEnd.y*LineStart.x)
            / delta.magnitude();
    }
    else
    {
        // Endpoint distances
        double d1 = P.distance(LineStart);
        double d2 = P.distance(LineEnd);
        if(d1 < d2)
            d = d1;
        else
            d = d2;
    }
    return d;
}

inline double distance_from_rect(const Coordinate2& p, const Coordinate2& anchor, double width, double height)
{
    using std::min;
    double left_dist =
        distance_from_line(p,anchor,Coordinate2(anchor.x,anchor.y + height));
    double top_dist =
        distance_from_line(p,Coordinate2(anchor.x,anchor.y+height),Coordinate2(anchor.x+width,anchor.y + height));
    double right_dist =
        distance_from_line(p,Coordinate2(anchor.x+width,anchor.y+height),Coordinate2(anchor.x+width,anchor.y));
    double bottom_dist =
        distance_from_line(p,Coordinate2(anchor.x,anchor.y),Coordinate2(anchor.x,anchor.y));
    return min(min(left_dist,top_dist),min(right_dist,bottom_dist));
}

inline double distance_from_circle(Coordinate2 p, Coordinate2 center, double radius)
{
    double dist = p.distance(center);
    return radius - dist;
}

inline double distance_from_arc(Coordinate2 p, Coordinate2 center, double radius, double start_angle_d, double end_angle_d)
{
    using std::abs;
    using std::cos;
    using std::sin;
    using std::atan2;

    double distance = 0;
    if(start_angle_d != end_angle_d)
    {
        // Normalize and get endpoints
        if(start_angle_d > end_angle_d) // Make sure start is lower angle
        {
            double temp_ang = start_angle_d;
            start_angle_d = end_angle_d;
            end_angle_d = temp_ang;
        }

        double start_angle = deg2rad(start_angle_d);
        double end_angle = deg2rad(end_angle_d);

        while(start_angle < 0)
        {
            start_angle += 2*pi;
            end_angle += 2*pi;
        }

        if(abs(end_angle - start_angle) < 2*pi) // If the arc is not a full circle
        {
            // Start and end points on arc
            Coordinate2 startpoint(center.x + radius*cos(start_angle),center.y + radius*sin(start_angle));
            Coordinate2 endpoint(center.x + radius*cos(end_angle),center.y + radius*sin(end_angle));

            // Get angle from arc center to point
            double p_angle = atan2(p.y-center.y,p.x-center.x);
            if(p_angle < 0) // Normalize to 0 to 2pi
                p_angle = p_angle + 2*pi;

            // Check for angle inclusion
            bool incl=false,cwside=false,ccwside=false;
            if(p_angle > start_angle && p_angle < end_angle)
                incl = true; // Angle of p included
            else if(p_angle < start_angle)
                cwside = true; // Angle of p is clockwise from arc
            else if(p_angle > end_angle)
                ccwside = true; // Angle of p is counterclockwise from arc

            // Compute distance
            if(incl)
                distance = abs(radius - p.distance(center));
            else if(cwside)
                distance = p.distance(startpoint);
            else if(ccwside)
                distance = p.distance(endpoint);
            else
                distance = -1; // Error
        }
        else // Arc is circle, use circle distance
            distance = abs(radius - p.distance(center));
    }
    else // Angles are the same, only one point
    {
        if(start_angle_d > end_angle_d) // Make sure start is lower angle
        {
            double temp_ang = start_angle_d;
            //start_angle_d = end_angle_d;
            end_angle_d = temp_ang;
        }

        double end_angle = deg2rad(end_angle_d);
        Coordinate2 anglepoint(center.x + radius*cos(end_angle),center.y + radius*sin(end_angle));
        distance = p.distance(anglepoint);
    }
    return distance;
}


inline bool collinear(Coordinate2 pt1, Coordinate2 pt2, Coordinate2 pt3,double tolerance=0.1)
{
    // Determine if three points lie on the same line (max orthogonal distance = tolerance)
    double det = std::abs((pt2.x-pt1.x)*(pt3.y-pt1.y)-(pt2.y-pt1.y)*(pt3.x-pt1.x));
    if(0.5*det < tolerance) return true;
    else return false;
}

inline std::vector<Coordinate2> sort_by_distance(std::vector<Coordinate2> pts, const Coordinate2& center)
{
    // Return pts, but sorted from nearest to farthest from center
    // Intended only for small sets of points
    std::vector<Coordinate2> ret;
    unsigned long orig_sz = pts.size();
    if(orig_sz > 0)
    {
        unsigned long sz = 0;
        while(sz != orig_sz)
        {
            auto closest_itr = pts.begin();
            double min_dist = closest_itr->distance(center);
            for(auto pt_itr = pts.begin(); pt_itr != pts.end(); ++pt_itr)
            {
                if(pt_itr->distance(center) < min_dist)
                {
                    closest_itr = pt_itr;
                    min_dist = closest_itr->distance(center);
                }
            }
            ret.push_back(*closest_itr);
            pts.erase(closest_itr);
            sz = ret.size();
        }
    }
    return ret;
}


#endif // COORDINATE2_H
