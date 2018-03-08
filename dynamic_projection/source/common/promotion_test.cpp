
template <typename T1, typename T2>
struct promotion_traits {
  static T1 t1;
  static T2 t2;  
  typedef typeof(t1 + t2) promoted_type;
};

template <class base_class>
class image {
public:
  typedef base_class base_class_t;
};

template <class image_class>
class composite {
  typedef typename image_class::base_class_t base_class_t;
  void foo(){
    base_class_t var;
  }
};

template <typename T1, typename T2>
image<typename promotion_traits<T1,T2>::promoted_type>
operator+(const image<T1> &i1, const image<T2> &i2){
  typedef typename promotion_traits<T1,T2>::promoted_type result_t;
  if (i1.getRows() != i2.getRows() ||
      i1.getCols() != i2.getCols()){
    // error
  }
  Image<result_t> result(i1.getRows(), i2.getCols());
  // note need reference counting to make Image<> template work for these

  return result;
}

//void operator+(image<int> b, image<float> c){
//}

int main(){
  composite<image<int> > a;
  image<int> b;
  image<float> c;
  image<float> d = b + c;
  return 0;
}
