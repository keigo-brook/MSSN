extern crate netopt;
extern crate mqttc;
extern crate mqtt3;
extern crate term;

use std::io;
use std::fmt;
use std::process::exit;
use netopt::{NetworkOptions, SslContext};
use mqtt3::{LastWill, SubscribeTopic, QoS, Protocol};
use mqttc::{PubSub, ClientOptions, ReconnectMethod, Error};
use mqttc::store;
use std::time::Duration;
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



fn print_legend() {
    let mut t = term::stdout().unwrap();
    t.fg(term::color::BRIGHT_GREEN).unwrap();
    write!(t, "        Legend ").unwrap();
    t.fg(term::color::BRIGHT_CYAN).unwrap();
    write!(t, "QoS 1 ").unwrap();
    t.fg(term::color::BRIGHT_MAGENTA).unwrap();
    write!(t, "QoS 2 ").unwrap();
    t.fg(term::color::BRIGHT_BLUE).unwrap();
    writeln!(t, "QoS 3 ").unwrap();
    t.fg(term::color::BRIGHT_GREEN).unwrap();
    // writeln!(t, "Network").unwrap();
    // t.reset().unwrap();
}

fn print_topics(topics: &[SubscribeTopic]) {
    let mut t = term::stdout().unwrap();
    t.fg(term::color::BRIGHT_GREEN).unwrap();
    write!(t, "     Subscribe ").unwrap();
    for topic in topics {
        t.fg(match topic.qos {
                QoS::AtMostOnce => term::color::BRIGHT_CYAN,
                QoS::AtLeastOnce => term::color::BRIGHT_MAGENTA,
                QoS::ExactlyOnce => term::color::BRIGHT_BLUE,
            })
            .unwrap();
        write!(t, "{} ", topic.topic_path).unwrap();
    }
    t.reset().unwrap();
    writeln!(t, "").unwrap();
}

fn print_message<T: AsRef<str>, M: AsRef<str>>(title: T, message: M, color: u16) {
    let mut t = term::stdout().unwrap();
    // t.fg(color).unwrap();
    // write!(t, "{:>14} ", title.as_ref()).unwrap();
    // t.reset().unwrap();
    writeln!(t, "{}", message.as_ref()).unwrap();
    t.reset().unwrap();
}

fn print_error<M: AsRef<str>>(message: M) {
    print_message("Error", message, term::color::BRIGHT_RED);
}








impl Point2 {
    fn read_point() -> Option<Point2> {
        let mut buffer = String::new();
        io::stdin().read_line(&mut buffer).expect("Failed to get_xy");
        let xy: Vec<i32> = buffer.trim()
            .split(' ')
            .map(|s| s.parse().unwrap())
            .collect();
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

fn read_data(bg_data: &Vec<Point2>, new_data: &[&str]) -> Option<Vec<Point2>> {
    let mut data = vec![];
    for i in 0..1081 {
        let p: Vec<i32> = new_data[i].split(' ').map(|s| s.parse::<i32>().unwrap()).collect();
        if p.len() < 2 {
            return None;
        }
        let mut point = Point2 { x: p[0], y: p[1] };
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

    let netopt = NetworkOptions::new();
    let mut opts = ClientOptions::new();
    opts.set_reconnect(ReconnectMethod::ReconnectAfter(Duration::from_secs(1)));
    let mut client = opts.connect("127.0.0.1:1883", netopt).expect("Can't connect to server");

    client.subscribe("/LS/data").unwrap();
    loop {
        match client.await() {
            Ok(some_message) => {
                if let Some(ref message) = some_message {
                    let color = match message.qos {
                        QoS::AtMostOnce => term::color::BRIGHT_CYAN,
                        QoS::AtLeastOnce => term::color::BRIGHT_MAGENTA,
                        QoS::ExactlyOnce => term::color::BRIGHT_BLUE,
                    };
                    let payload = match String::from_utf8((*message.payload).clone()) {
                        Ok(payload) => payload,
                        Err(_) => {
                            format!("payload did not contain valid UTF-8 ({} bytes)",
                                    message.payload.len())
                        }
                    };

                    // print_message(&message.topic.path, payload, color);
                    let data: Vec<&str> = payload.trim().split('\n').collect();
                    if bg_data.len() == 0 {
                        let t: i32 = match data[0].parse() {
                            Ok(t) => t,
                            Err(_) => {
                                continue;
                            }
                        };
                        for i in 1..1082 {
                            let p: Vec<i32> =
                                data[i].split(' ').map(|s| s.parse::<i32>().unwrap()).collect();
                            bg_data.push(Point2 { x: p[0], y: p[1] });
                        }
                    } else {
                        for i in 0..(data.len() / 1082) {
                            let t: i32 = match data[0 + i * 1082].parse() {
                                Ok(t) => t,
                                Err(_) => {
                                    continue;
                                }
                            };
                            if let Some(data) = read_data(&bg_data,
                                                          &data[(1 + i * 1082)..(1082 +
                                                                                 i * 1082)]) {
                                let mut clusters = Clusters { points: vec![] };
                                clusters.calc_clusters(data);
                                println!("{} {}", t, clusters.points.len());
                                clusters.print_points();
                            }
                        }


                    }

                    if message.qos == QoS::ExactlyOnce {
                        let _ = client.complete(message.pid.unwrap());
                    }

                }
            }
            Err(e) => {
                match e {
                    Error::UnhandledPuback(_) => print_error("unhandled puback"),
                    Error::UnhandledPubrec(_) => print_error("unhandled pubrec"),
                    Error::UnhandledPubrel(_) => print_error("unhandled pubrel"),
                    Error::UnhandledPubcomp(_) => print_error("unhandled pubcomp"),
                    Error::Mqtt(ref err) => {
                        match *err {
                            mqtt3::Error::TopicNameMustNotContainNonUtf8 => {
                                print_error("topic name contains non-UTF-8 characters")
                            }
                            mqtt3::Error::TopicNameMustNotContainWildcard => {
                                print_error("topic name contains wildcard")
                            }
                            _ => {
                                print_error(format!("{:?}", e));
                                exit(64);
                            }
                        }
                    }
                    Error::Storage(ref err) => {
                        match *err {
                            store::Error::NotFound(pid) => {
                                // we have lost something
                                let _ = client.complete(pid);
                            }
                            store::Error::Unavailable(_) => {
                                // do nothing, just wait next pubrel
                            }
                        }
                    }
                    Error::Disconnected | Error::ConnectionAbort => {
                        exit(64);
                    }
                    e => {
                        print_error(format!("{:?}", e));
                        client.terminate();
                        exit(64);
                    }
                }
            }
        }
    }

    // 1081.times { bgdata.push(gets.split.map(&:to_i)) }
    // let mut buffer = String::new();
    // loop {
    //     io::stdin().read_line(&mut buffer).expect("Failed to get timestamp");
    //     let t: Vec<i32> = buffer.trim().split(' ').map(|s| s.parse().unwrap()).collect();
    //     buffer.clear();
    //     if t.len() == 1 {
    //         println!("{}", t[0]);
    //         break;
    //     }
    // }

    // for _ in 0..1081 {
    //     if let Some(p) = Point2::read_point() {
    //         bg_data.push(p);
    //     } else {
    //         panic!("ERROR: read background data");
    //     }
    // }

    // loop {
    //     buffer.clear();
    //     match io::stdin().read_line(&mut buffer) {
    //         Ok(l) if l == 0 => break, // EOF
    //         Ok(_) => {},
    //         Err(_) => panic!("Failed to read line"),
    //     }
    //     let t: i32 = match buffer.trim().parse() {
    //         Ok(t) => t,
    //         Err(_) => {
    //             continue;
    //         }
    //     };
    //     if let Some(data) = read_data(&bg_data) {
    //         // clusters = Clustering.calc_culsters(data)
    //         // let mut clusters = Clusters { points: vec![] };
    //         // clusters.calc_clusters(data);
    //         // println!("{} {}", t, clusters.points.len());
    //         println!("{}", t);
    //         // clusters.print_points();
    //     }
    // }
}
