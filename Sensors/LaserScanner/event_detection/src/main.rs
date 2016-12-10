use std::io;

fn read_vec<T: std::str::FromStr>() -> Vec<T> {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    line.trim().split_whitespace().map(|s| s.parse().ok().unwrap()).collect()
}

#[derive(Copy, Clone)]
struct Point2 {
    x: f32,
    y: f32,
}

struct Human {
    missed: i32,
    history: Vec<(i32, Point2)>,
    detected: bool,
}

impl Human {
    fn new() -> Human {
        Human {
            missed: 0,
            history: vec![],
            detected: true,
        }
    }

    fn near(&self, p: Point2) -> bool {
        if self.distance(p) < 150.0 {
            true
        } else {
            false
        }
    }

    fn distance(&self, p: Point2) -> f32 {
        let last_p = self.history.last().unwrap().1;
        ((last_p.x - p.x) * (last_p.x - p.x) + (last_p.y - p.y) * (last_p.y - p.y)).sqrt()
    }

    fn push_if_near(&mut self, t: i32, p: Point2) -> bool {
        if self.near(p) {
            self.history.push((t, p));
            self.missed = 0;
            self.detected = true;
            true
        } else {
            false
        }
    }

    fn in_area(&self) -> bool {
        let last_p = self.history.last().unwrap().1;
        0.0 <= last_p.x && last_p.x <= 2000.0 && -5000.0 <= last_p.y && last_p.y <= -1000.0
    }

    fn moved(&self) -> bool {
        for i in 1..self.history.len() {
            if self.distance(self.history[self.history.len() - i].1) > 25.0 {
                return true;
            } else if i > 4 {
                return false;
            }
        }
        return false;
    }
}

struct EventDetection;
impl EventDetection {
    fn detect(sequences: Vec<(i32, Vec<Point2>)>) {
        let mut humans: Vec<Human> = vec![];
        let mut seq_result = 0;
        for i in 0..sequences.len() {
	    let t = sequences[i].0;
            let ref sequence_data = sequences[i].1;
            for mut h in &mut humans {
                h.detected = false;
            }
            for point in sequence_data {
                let mut used = false;
                for mut h in &mut humans {
                    if h.push_if_near(t, *point) {
                        used = true;
                    }
                }
                if !used {
                    let mut new_man = Human::new();
                    new_man.history.push((t, Point2 {
                        x: point.x,
                        y: point.y,
                    }));
                    humans.push(new_man);
                }
            }

            for mut h in &mut humans {
                if !h.detected {
                    h.missed += 1;
                }
            }
            humans.retain(|p| p.missed < 4);
            for h in &humans {
                if !h.in_area() {
                    continue;
                }
                seq_result += if h.moved() {
                    1
                } else {
                    2
                };
            }
        }
        let event = seq_result as f32 / sequences.len() as f32;
        let result = if event >= 1.5 {
            2
        } else if event >= 0.5 {
            1
        } else {
            0
        };
        println!(" {}", result);
    }
}

fn main() {
    let mut sequences = vec![];
    let mut buffer = String::new();
    let mut last_detected_at = 0;
    loop {
        match io::stdin().read_line(&mut buffer) {
            Ok(l) if l == 0 => break,
            Ok(_) => {}
            Err(_) => panic!("Failed to read line"),
        }
        let t_n: Vec<i32> = buffer.trim().split(' ').map(|s| s.parse().unwrap()).collect();
        let (t, n) = (t_n[0], t_n[1]);
        buffer.clear();
        let mut data = vec![];
        for _ in 0..n {
            let xy: Vec<f32> = read_vec();
            let p = Point2 {
                x: xy[0],
                y: xy[1],
            };
            data.push(p);
        }
        sequences.push((t, data));
        if t - last_detected_at >= 1000 {
            print!("{} {}", t, sequences.len());
            EventDetection::detect(sequences);
            sequences = vec![];
            last_detected_at = t;
        }
    }
}
