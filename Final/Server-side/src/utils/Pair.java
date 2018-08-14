package utils;


public class Pair<L,R>{

	private  L left;
	private  R right;

	public Pair(L left, R right) {
	    this.left = left;
	    this.right = right;
	 }

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((left == null) ? 0 : left.hashCode());
		result = prime * result + ((right == null) ? 0 : right.hashCode());
		return result;
	}



	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		@SuppressWarnings("unchecked")
		Pair<L, R> other = (Pair<L,R>) obj;
		if (left == null) {
			if (other.left != null)
				return false;
		} else if (!left.equals(other.left))
			return false;
		return true;
	}

	public synchronized R getValue() {
		return right;
	}
	
	public synchronized L getKey() {
		return left;
	}

}
