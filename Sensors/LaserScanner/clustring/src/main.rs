use std::io;
use std::fmt;

#[derive(Debug, Copy, Clone)]
struct Point2 {
    x: i32,
    y: i32,
}

impl fmt::Display for Point2 {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "x: {}, y: {}", self.x, self.y)
    }
}

impl Point2 {
    fn read_point() -> Option<Point2> {
        let mut buffer = String::new();
        io::stdin().read_line(&mut buffer).expect("Failed to get_xy");
        let xy: Vec<i32> = buffer.trim()
            .split(' ')
            .map(|s| s.parse().unwrap())
            .collect();
        buffer.clear();
        if xy.len() < 2 {
            return None;
        } 
        Some(Point2 {
            x: xy[0],
            y: xy[1],
        })
    }
}

#[derive(Copy, Clone)]
struct Cluster {
    x: f32,
    y: f32,
    size: i32,
}

struct Clusters {
    points: Vec<Cluster>,
}

impl Clusters {
    fn distance(c1: Cluster, c2: Cluster) -> f32 {
        (((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y)) as f32).sqrt()
    }

    fn calc_clusters(&mut self, data: Vec<Point2>) {
        for d in data {
            match (d.x, d.y) {
                (0, 0) => continue,
                _ => {
                    self.points.push(Cluster {
                        x: d.x as f32,
                        y: d.y as f32,
                        size: 1,
                    })
                }
            }
        }
        loop {
            self.points.sort_by(|a, b| a.x.partial_cmp(&b.x).unwrap());
            match Clusters::closest_pair(&mut self.points) {
                (None, None, _) => break, // no valid pair
                (Some(x), Some(y), n) => {
                    if n > 800.0 {
                        break;
                    } else {
                        self.merge(x, y);
                    }
                }
                _ => panic!("Error! Find closest pair"),
            }
        }
        self.points.retain(|&p| p.size > 9);
    }

    fn closest_pair(points: &mut Vec<Cluster>) -> (Option<Cluster>, Option<Cluster>, f32) {
        if points.len() < 2 {
            return (None, None, std::f32::INFINITY);
        }
        let mut c1;
        let mut c2;
        let mut d_min;
        {
            let (l, r) = points.split_at(points.len() / 2);
            let (lc1, lc2, ld_min) = Clusters::closest_pair(&mut l.to_vec());
            let (rc1, rc2, rd_min) = Clusters::closest_pair(&mut r.to_vec());
            if ld_min > rd_min {
                c1 = rc1;
                c2 = rc2;
                d_min = rd_min;
            } else {
                c1 = lc1;
                c2 = lc2;
                d_min = ld_min;
            };
        }
        let m = points[points.len() / 2];
        points.sort_by(|a, b| a.y.partial_cmp(&b.y).unwrap());
        let mut b: Vec<Cluster> = Vec::new();
        for c in points {
            if (c.x - m.x).abs() as f32 >= d_min {
                continue;
            }
            for rb in b.iter().rev() {
                let d = Clusters::distance(*c, *rb);
                let dy = c.y - rb.y;
                if dy as f32 >= d_min {
                    break;
                } else if d_min > d {
                    c1 = Some(*c);
                    c2 = Some(*rb);
                    d_min = d;
                }
            }
            b.push(*c);
        }
        (c1, c2, d_min)
    }

    fn merge(&mut self, c1: Cluster, c2: Cluster) {
        let new_size = c1.size + c2.size;
        let new_x = (c1.x * c1.size as f32 + c2.x * c2.size as f32) / new_size as f32;
        let new_y = (c1.y * c1.size as f32 + c2.y * c2.size as f32) / new_size as f32;
        self.points.retain(|&p| (p.x != c1.x || p.y != c1.y) && (p.x != c2.x || p.y != c2.y));
        self.points.push(Cluster {
            x: new_x,
            y: new_y,
            size: new_size,
        });
    }

    fn print_points(self) {
        for p in self.points {
            println!("{} {}", p.x, p.y);
        }
    }
}

fn read_data(bg_data: &Vec<Point2>) -> Option<Vec<Point2>> {
    let mut data = vec![]; 
    for i in 0..1081 {
        let mut point;
        if let Some(p) = Point2::read_point() {
            point = p;
        } else {
            return None;
        }
        if (point.x - bg_data[i].x).abs() <= 100 && (point.y - bg_data[i].y).abs() <= 100 {
            point.x = 0;
            point.y = 0;
        }
        data.push(point);
    }
    return Some(data);
}

fn main() {
    let mut bg_data = vec![];

    // 1081.times { bgdata.push(gets.split.map(&:to_i)) }
    let mut buffer = String::new();
    io::stdin().read_line(&mut buffer).expect("Failed to get timestamp");

    buffer.clear();
    for _ in 0..1081 {
        if let Some(p) = Point2::read_point() {
            bg_data.push(p);
        } else {
            panic!("ERROR: read background data");
        }
    }

    loop {
        match io::stdin().read_line(&mut buffer) {
            Ok(l) if l == 0 => break, // EOF
            Ok(_) => {},
            Err(_) => panic!("Failed to read line"),
        }
        let t: i32 = match buffer.trim().parse() {
            Ok(t) => t,
            Err(_) => { 
                buffer.clear(); 
                continue;
            }
        };
        buffer.clear();
        if let Some(data) = read_data(&bg_data) {
            // clusters = Clustering.calc_culsters(data)
            let mut clusters = Clusters { points: vec![] };
            clusters.calc_clusters(data);
            println!("{} {}", t, clusters.points.len());
            clusters.print_points();
        }
    }
}
