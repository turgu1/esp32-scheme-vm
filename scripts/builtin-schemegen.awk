END {
	for(i = 0; i <= max_idx; i++) {
    if (!pr[i, "scheme_name"]) {
      print i " is not defining a builtin procedure." > "/dev/stderr"
    }
    else {
		  print "(define-builtin " pr[i, "scheme_name"] " " pr[i, "arguments"] " " i " " pr[i, "scheme_options"] ")"
    }
	}
}
